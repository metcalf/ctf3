package cluster

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"github.com/gorilla/mux"
	"github.com/metcalf/ctf3/level4/db"
	"github.com/metcalf/ctf3/level4/debuglog"
	"github.com/metcalf/ctf3/level4/transport"
	"github.com/metcalf/raft"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"path/filepath"
	"time"
)

type EncodableCommand interface {
	raft.Command
	raft.CommandEncoder
}

type RequestHandler func(do CommandHandler, mux *mux.Router) error
type CommandHandler func(cmd EncodableCommand) (int, error)

type Cluster struct {
	listen     string
	path       string
	name       string
	handler    RequestHandler
	raftServer raft.Server
	router     *mux.Router
	context    interface{}
	client     *transport.Client
}

func New(path string, listen string, handler RequestHandler, context interface{}) (*Cluster, error) {
	c := &Cluster{
		listen:  listen,
		path:    path,
		handler: handler,
		router:  mux.NewRouter(),
		context: context,
		client:  transport.NewClient(),
	}

	// Read existing name or generate a new one.
	if b, err := ioutil.ReadFile(filepath.Join(path, "name")); err == nil {
		c.name = string(b)
	} else {
		c.name = listen
	}

	return c, nil
}

// Starts the server.
func (c *Cluster) ListenAndServe(leader string) error {
	var err error

	log.Printf("Initializing Raft Server: %s", c.path)

	// Initialize and start Raft server.
	transporter := transport.NewHTTPTransporter("/raft")

	c.raftServer, err = raft.NewServer(c.name, c.path, transporter, nil, c.context, "")
	if err != nil {
		return err
	}
	transporter.Install(c.raftServer, c)
	c.raftServer.Start()

	if !c.raftServer.IsLogEmpty() {
		log.Println("Recovered from log")
	} else if leader != "" {
		// Join to leader if specified.

		log.Printf("Attempting to join leader: %s", leader)

		// Give the master time to boot
		time.Sleep(50 * time.Millisecond)

		if err := c.Join(leader); err != nil {
			return err
		}

	} else {
		// Initialize the server by joining itself.
		log.Println("Initializing new cluster")

		_, err := c.raftServer.Do(&raft.DefaultJoinCommand{
			Name:             c.raftServer.Name(),
			ConnectionString: c.connectionString(),
		})
		if err != nil {
			return err
		}

	}

	// Initialize and start HTTP server.
	httpServer := &http.Server{
		Handler: c.router,
	}

	c.router.HandleFunc("/join", c.joinHandler).Methods("POST")
	c.router.HandleFunc("/do/{command}", c.doHandler).Methods("POST")

	// Start Unix transport
	l, err := transport.Listen(c.listen)
	if err != nil {
		return err
	}

	log.Println("Initializing HTTP server")
	c.handler(c.Do, c.router)

	return httpServer.Serve(l)
}

// This is a hack around Gorilla mux not providing the correct net/http
// HandleFunc() interface.
func (c *Cluster) HandleFunc(pattern string, handler func(http.ResponseWriter, *http.Request)) {
	c.router.HandleFunc(pattern, handler)
}

// Joins to the leader of an existing cluster.
func (c *Cluster) Join(leader string) error {
	command := &raft.DefaultJoinCommand{
		Name:             c.raftServer.Name(),
		ConnectionString: c.connectionString(),
	}

	var b bytes.Buffer
	json.NewEncoder(&b).Encode(command)
	jsonReader := bytes.NewReader(b.Bytes())

	cs, err := transport.Encode(leader)
	if err != nil {
		return err
	}

	log.Printf("Sending join command with contents: %s", b.Bytes())
	for {
		_, err := c.client.SafePost(cs, "/join", jsonReader)

		if err != nil {
			log.Printf("Unable to join cluster: %s", err)
			time.Sleep(500 * time.Millisecond)
			jsonReader.Seek(0, 0)
		} else {
			break
		}
	}

	return nil
}

func (c *Cluster) connectionString() string {
	conn, err := transport.Encode(c.listen)
	if err != nil {
		log.Fatalf("Error determining connectionString: %s", err)
	}

	return conn
}

func (c *Cluster) joinHandler(w http.ResponseWriter, req *http.Request) {
	debuglog.Debugf("Received join request (I am %s)", c.raftServer.State())

	command := &raft.DefaultJoinCommand{}

	err := json.NewDecoder(req.Body).Decode(&command)
	if err != nil && err != io.EOF {
		log.Printf("Could not decode join command: %s", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	if command.ConnectionString == "" {
		cErr := fmt.Errorf("Join request with empty connection string.  JSON err was: %s", err)
		log.Print(cErr)
		http.Error(w, cErr.Error(), http.StatusInternalServerError)
		return
	}

	debuglog.Debugf("Processing join request from %s", command.ConnectionString)
	if _, err := c.raftServer.Do(command); err != nil {
		log.Printf("Could not execute join command: %s", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	debuglog.Debugf("Processed join request from %s", command.ConnectionString)
}

func (c *Cluster) Do(cmd EncodableCommand) (int, error) {
	var index uint32

	switch c.raftServer.State() {
	case raft.Stopped:
		return 0, fmt.Errorf("Raft server is currently stopped")
	case raft.Leader:
		debuglog.Debugln("I'm the leader, executing action locally")
		index, err := c.raftServer.Do(cmd)
		if err != nil {
			return 0, err
		} else {
			return index.(int), err
		}
	default:
		if c.raftServer.Leader() == "" {
			return 0, fmt.Errorf("No leader elected")
		}
		debuglog.Debugf("Forwarding Action to the leader (in state %s): %s",
			c.raftServer.State(), c.raftServer.Leader())
		var cmdBuf bytes.Buffer
		cmd.Encode(&cmdBuf)

		leader := c.raftServer.Peers()[c.raftServer.Leader()]
		if leader == nil {
			return 0, fmt.Errorf("Unable to find leader `%s` in peers list",
				c.raftServer.Leader())
		}

		resp, err := c.client.SafePost(
			leader.ConnectionString,
			fmt.Sprintf("/do/%s", cmd.CommandName()),
			&cmdBuf)
		if err != nil {
			return 0, err
		}

		if err := binary.Read(resp, binary.BigEndian, &index); err != nil {
			return 0, err
		}

		return int(index), nil
	}
}

func (c *Cluster) doHandler(w http.ResponseWriter, req *http.Request) {
	vars := mux.Vars(req)
	cmdName := vars["command"]

	if cmdName != "action" {
		err := fmt.Sprintf("Currently only support forwarding actions, got: %s", cmdName)
		log.Printf(err)
		http.Error(w, err, http.StatusInternalServerError)
		return
	}

	cmd := &db.Action{}

	if err := cmd.Decode(req.Body); err != nil {
		log.Printf("Error decoding forwarded command: %s", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	index, err := c.raftServer.Do(cmd)
	if err != nil {
		log.Printf("Error applying forwarded command: %s", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	if err := binary.Write(w, binary.BigEndian, uint32(index.(int))); err != nil {
		log.Printf("Error writing response to forwarded command: %s", err)
	}
}
