package server

import (
	"bytes"
	"encoding/binary"
	"github.com/goraft/raft"
	"github.com/narced133/ctf3/level4/cluster"
)

type Action struct {
	rowId        uint32
	inc          uint8
	favoriteWord string
}

func (a *Action) CommandName() string {
	"action"
}

func (a *Action) Apply(context raft.Context) ([]*Action, error) {
	return nil, nil
}

/*func (action *Action) Encode() ([]byte, error) {
	buf := new(bytes.Buffer)

	if err := binary.Write(buf, binary.BigEndian, action.rowId); err != nil {
		return nil, err
	}
	if err := binary.Write(buf, binary.BigEndian, action.inc); err != nil {
		return nil, err
	}
	if _, err := buf.Write([]byte(action.favoriteWord)); err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (action *Action) Decode(data []byte) error {
	reader := bytes.NewReader(data)
	var inc uint8
	var rowId uint32
	var wordData []byte

	if err := binary.Read(reader, binary.BigEndian, &rowId); err != nil {
		return err
	}
	if err := binary.Read(reader, binary.BigEndian, &inc); err != nil {
		return err
	}
	if _, err := reader.Read(wordData); err != nil {
		return err
	}

	action.rowId = rowId
	action.inc = inc
	action.favoriteWord = string(wordData)
	return nil
}*/
