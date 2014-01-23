package server

import (
	"fmt"
	"github.com/gorilla/mux"
	"github.com/narced133/ctf3/level4/cluster"
	"github.com/narced133/ctf3/level4/debuglog"
	"io/ioutil"
	"log"
	"net/http"
	"regexp"
	"strconv"
	"strings"
)

const rowCount = 4

// Create a cluster or join it (passing the master)
// New(request_chan, update_chan, <join addr>)

type Server struct {
	rowNames [rowCount]string
	do       cluster.CommandHandler
}

type Row struct {
	name         string
	friendCount  uint8
	requestCount uint16
	favoriteWord string
}

func New() (*Server, error) {
	return &Server{}, nil
}

// TODO: This shouldn't know about the cluster, just a callback to send
// messages into the cluster
func (s *Server) ListenAndServe(do cluster.CommandHandler, mux *mux.Router) error {
	s.do = do
	mux.HandleFunc("/sql", s.sqlHandler).Methods("POST")

	return nil
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

		s.insertHandler(w, strings.Split(matches[1], "\"), (\""))
	} else {
		err := fmt.Errorf("Body did not match any query formats.  Body was:\n%s", query)
		log.Printf(err.Error())
		http.Error(w, err.Error(), http.StatusBadRequest)
	}
}

func (s *Server) insertHandler(w http.ResponseWriter, names []string) {
	if len(names) != rowCount {
		log.Fatalf("Got %d names but only expect %d!", len(names), rowCount)
	}

	for i, name := range names {
		s.rowNames[i] = name
	}
}

func (s *Server) updateHandler(w http.ResponseWriter, name string, inc uint8, word string) {
	rowId := func() int {
		for i, rName := range s.rowNames {
			if rName == name {
				return i
			}
		}
		return -1
	}()

	if rowId < 0 {
		err := fmt.Sprintf("Could not find name %s.  I only know about %s.",
			name, strings.Join(s.rowNames[:], ", "))

		log.Print(err)
		http.Error(w, err, http.StatusBadRequest)
		return
	}

	action := &Action{
		rowId:        uint32(rowId),
		inc:          inc,
		favoriteWord: word,
	}

	actions, err := s.do(Action)
	if err != nil {
		log.Print(err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	var responseData [rowCount]*Row
	var responseLines [rowCount]string

	for i, name := range s.rowNames {
		responseData[i] = &Row{
			name:         name,
			friendCount:  0,
			requestCount: 0,
			favoriteWord: "",
		}
	}

	// TODO: Abstract the encoding and decoding
	for action := range actions {
		row := responseData[action.rowId]

		row.friendCount += action.inc
		row.requestCount += 1
		row.favoriteWord = action.favoriteWord
	}

	for i, row := range responseData {
		responseLines[i] = fmt.Sprintf("%s|%d|%d|%s",
			row.name,
			row.friendCount,
			row.requestCount,
			row.favoriteWord)
	}

	resp := fmt.Sprintf("SequenceNumber: %d\n%s",
		commit.Index(), strings.Join(responseLines[:], "\n"))

	w.Write([]byte(resp))
}
