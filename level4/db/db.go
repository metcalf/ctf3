package db

import (
	"fmt"
	"github.com/goraft/raft"
	"github.com/metcalf/ctf3/level4/debuglog"
	"sync"
)

const rowCount = 5

type DBContext interface {
	DB() *DB
}

type AsyncResponse struct {
	Data  []*Row
	Error error
}

type DB struct {
	actions  []*Action
	rowNames [rowCount]string
	mutex    sync.RWMutex
	onAppend *sync.Cond
}

func New() *DB {
	// Setup commands.
	raft.RegisterCommand(&Action{})

	db := &DB{
		rowNames: [rowCount]string{"siddarth", "gdb", "christian", "andy", "carl"},
	}

	db.onAppend = sync.NewCond(&db.mutex)

	return db
}

func (db *DB) SetNames(names []string) error {
	if len(names) != rowCount {
		return fmt.Errorf("Got %d names but only expect %d!", len(names), rowCount)
	}

	for i, name := range names {
		if db.rowNames[i] != name {
			return fmt.Errorf("Name at index %d was %s but expected %s",
				i, name, db.rowNames[i])
		}
	}

	return nil
}

func (db *DB) Get(index int) []*Row {
	db.mutex.RLock()
	defer db.mutex.RUnlock()

	var responseData [rowCount]*Row
	var actions []*Action

	if index >= 0 {
		actions = db.actions[:index]
	} else {
		actions = db.actions
	}

	for i, name := range db.rowNames {
		responseData[i] = &Row{
			name:         name,
			friendCount:  0,
			requestCount: 0,
			favoriteWord: "",
		}
	}

	for _, action := range actions {
		row := responseData[action.rowId]

		row.friendCount += action.inc
		if action.inc > 0 {
			row.requestCount += 1
		}
		row.favoriteWord = action.favoriteWord
	}

	return responseData[:]
}

func (db *DB) GetWhenReady(index int) chan *AsyncResponse {
	ch := make(chan *AsyncResponse)

	go func() {
		db.mutex.Lock()
		for {
			debuglog.Debugf("Waiting for %d to be >= %d", len(db.actions), index)
			if len(db.actions) >= index {
				db.mutex.Unlock()
				debuglog.Debugln("DB ready, getting")
				ch <- &AsyncResponse{
					Data: db.Get(index),
				}
				return
			} else {
				debuglog.Debugln("DB not ready, waiting")
				db.onAppend.Wait()
			}
		}
	}()

	return ch
}

func (db *DB) Put(action *Action) int {
	db.mutex.Lock()
	defer db.mutex.Unlock()

	action.DebugLog("Storing")

	db.actions = append(db.actions, action)
	db.onAppend.Broadcast()

	return len(db.actions)
}

func (db *DB) RowNames() []string {
	return db.rowNames[:]
}
