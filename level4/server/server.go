package server

import (
	"fmt"
	"github.com/gorilla/mux"
	"github.com/metcalf/ctf3/level4/cluster"
	"github.com/metcalf/ctf3/level4/db"
	"github.com/metcalf/ctf3/level4/debuglog"
	"io/ioutil"
	"log"
	"net/http"
	"regexp"
	"strconv"
	"strings"
)

// Create a cluster or join it (passing the master)
// New(request_chan, update_chan, <join addr>)

type Server struct {
	do cluster.CommandHandler
	db *db.DB
}

func New() (*Server, error) {
	return &Server{
		db: db.New(),
	}, nil
}

func (s *Server) ListenAndServe(do cluster.CommandHandler, mux *mux.Router) error {
	s.do = do
	mux.HandleFunc("/sql", s.sqlHandler).Methods("POST")

	return nil
}

func (s *Server) DB() *db.DB {
	return s.db
}

var updateMatcher *regexp.Regexp = regexp.MustCompile(
	"UPDATE ctf3 SET friendCount=friendCount\\+(\\d+), requestCount=requestCount\\+1, favoriteWord=\"(\\w+)\" WHERE name=\"(\\w+)\"; SELECT \\* FROM ctf3;")
var insertMatcher *regexp.Regexp = regexp.MustCompile(
	"INSERT INTO ctf3 \\(name\\) VALUES \\(\"(.*)\"\\);")

func (s *Server) sqlHandler(w http.ResponseWriter, req *http.Request) {
	queryBytes, err := ioutil.ReadAll(req.Body)
	if err != nil {
		log.Printf("Couldn't read body: %s", err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	query := string(queryBytes)

	if matches := updateMatcher.FindStringSubmatch(query); matches != nil {
		debuglog.Debugf("Handling update query: %s", query)
		if inc, err := strconv.ParseUint(matches[1], 0, 8); err != nil {
			log.Printf("Could not parse '%d' to an integer (%s)",
				matches[1], err)
			http.Error(w, err.Error(), http.StatusBadRequest)
		} else {
			s.updateHandler(w, matches[3], uint8(inc), matches[2])
		}
	} else if matches := insertMatcher.FindStringSubmatch(query); matches != nil {
		debuglog.Debugf("Handling insert query: %s", query)
		return

		s.insertHandler(w, strings.Split(matches[1], "\"), (\""))
	} else {
		err := fmt.Errorf("Body did not match any query formats.  Body was:\n%s", query)
		log.Printf(err.Error())
		http.Error(w, err.Error(), http.StatusBadRequest)
	}
}

func (s *Server) insertHandler(w http.ResponseWriter, names []string) {
	if err := s.db.SetNames(names); err != nil {
		log.Fatal(err)
	}
}

func (s *Server) updateHandler(w http.ResponseWriter, name string, inc uint8, word string) {
	rowId := func() int {
		for i, rName := range s.db.RowNames() {
			if rName == name {
				return i
			}
		}
		return -1
	}()

	if rowId < 0 {
		err := fmt.Sprintf("Could not find name %s.  I only know about %s.",
			name, strings.Join(s.db.RowNames()[:], ", "))

		log.Print(err)
		http.Error(w, err, http.StatusBadRequest)
		return
	}

	action := db.NewAction(uint32(rowId), inc, word)

	action.DebugLog("Applying")

	index, err := s.do(action)
	if err != nil {
		log.Print(err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	response, ok := <-s.db.GetWhenReady(index)

	if !ok {
		log.Fatal("Error receiving on DB channel")
	}
	if err := response.Error; err != nil {
		log.Print(err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	var responseLines []string
	var seq uint16 = 0
	for _, row := range response.Data {
		responseLines = append(responseLines, row.Format())
		seq += row.RequestCount()
	}

	resp := fmt.Sprintf("SequenceNumber: %d\n%s",
		seq, strings.Join(responseLines[:], "\n"))

	w.Write([]byte(resp))
}
