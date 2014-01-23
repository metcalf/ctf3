package cluster

import (
	"bytes"
	"encoding/json"
	"fmt"
	"github.com/goraft/raft"
	"github.com/gorilla/mux"
	"github.com/narced133/ctf3/level4/transport"
	"log"
	"net"
	"net/http"
)

type RequestHandler func(do CommandHandler, mux *mux.Router) error
type CommandHandler func(cmd raft.Command) (interface{}, error)

type Cluster struct {
	listen     string
	path       string
	handler    RequestHandler
	raftServer raft.Server
	router     *mux.Router
}

func New(path string, listen string, handler RequestHandler) (*Cluster, error) {
	c := &Cluster{
		listen:  listen,
		path:    path,
		handler: handler,
		router:  mux.NewRouter(),
	}

	return c, nil
}

// Starts the server.
func (c *Cluster) ListenAndServe(leader string) error {
	var err error

	log.Printf("Initializing Raft Server: %s", c.path)

	// Initialize and start Raft server.
	transporter := raft.NewHTTPTransporter("/raft")
	c.raftServer, err = raft.NewServer("SQLCluster", c.path, transporter, nil, nil, "")
	if err != nil {
		log.Fatal(err)
	}
	transporter.Install(c.raftServer, c)
	c.raftServer.Start()

	if leader != "" {
		// Join to leader if specified.

		log.Println("Attempting to join leader:", leader)

		if !c.raftServer.IsLogEmpty() {
			log.Fatal("Cannot join with an existing log")
		}
		if err := c.Join(leader); err != nil {
			log.Fatal(err)
		}

	} else if c.raftServer.IsLogEmpty() {
		// Initialize the server by joining itself.

		log.Println("Initializing new cluster")

		_, err := c.raftServer.Do(&raft.DefaultJoinCommand{
			Name:             c.raftServer.Name(),
			ConnectionString: c.listen,
		})
		if err != nil {
			log.Fatal(err)
		}

	} else {
		log.Println("Recovered from log")
	}

	log.Println("Initializing HTTP server")

	// Initialize and start HTTP server.
	httpServer := &http.Server{
		Handler: c.router,
	}

	c.router.HandleFunc("/join", c.joinHandler).Methods("POST")

	c.handler(c.Do, c.router)

	// Start Unix transport
	l, err := transport.Listen(c.listen)
	if err != nil {
		log.Fatal(err)
	}

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
		ConnectionString: c.listen,
	}

	var b bytes.Buffer
	json.NewEncoder(&b).Encode(command)
	resp, err := http.Post(fmt.Sprintf("http://%s/join", leader), "application/json", &b)
	resp.Body.Close()
	if err != nil {
		return err
	}

	return nil
}

func (c *Cluster) joinHandler(w http.ResponseWriter, req *http.Request) {
	command := &raft.DefaultJoinCommand{}

	if err := json.NewDecoder(req.Body).Decode(&command); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	if _, err := c.raftServer.Do(command); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
}

func (c *Cluster) Do(cmd raft.Command) (interface{}, error) {
	return c.raftServer.Do(cmd)
}

func (c *Cluster) RequestListener() net.Listener {
	return nil
}
