package db

import (
	"encoding/binary"
	"github.com/metcalf/ctf3/level4/debuglog"
	"github.com/metcalf/raft"
	"io"
)

type Action struct {
	rowId        uint32
	inc          uint8
	favoriteWord string
}

func NewAction(rowId uint32, inc uint8, favoriteWord string) *Action {
	return &Action{
		rowId:        rowId,
		inc:          inc,
		favoriteWord: favoriteWord,
	}
}

func (a *Action) DebugLog(description string) {
	debuglog.Debugf(
		"%s action to increment row %d by %d "+
			"and set favorite word to %s",
		description,
		a.rowId,
		a.inc,
		a.favoriteWord)
}

func (a *Action) CommandName() string {
	return "action"
}

func (a *Action) Apply(context raft.Context) (interface{}, error) {
	return context.Server().Context().(DBContext).DB().Put(a), nil
}

func (action *Action) Encode(w io.Writer) error {
	strLen := uint8(len(action.favoriteWord))

	if err := binary.Write(w, binary.BigEndian, action.rowId); err != nil {
		return err
	}
	if err := binary.Write(w, binary.BigEndian, action.inc); err != nil {
		return err
	}
	if err := binary.Write(w, binary.BigEndian, strLen); err != nil {
		return err
	}
	if _, err := w.Write([]byte(action.favoriteWord)); err != nil {
		return err
	}

	return nil
}

func (action *Action) Decode(r io.Reader) error {
	var inc uint8
	var rowId uint32
	var strLen uint8

	if err := binary.Read(r, binary.BigEndian, &rowId); err != nil {
		return err
	}
	if err := binary.Read(r, binary.BigEndian, &inc); err != nil {
		return err
	}
	if err := binary.Read(r, binary.BigEndian, &strLen); err != nil {
		return err
	}

	wordData := make([]byte, strLen, strLen)
	if _, err := io.ReadFull(r, wordData); err != nil {
		debuglog.Debugf("Error reading wordData: %s", err)
		return err
	}

	action.rowId = rowId
	action.inc = inc
	action.favoriteWord = string(wordData)
	return nil
}
