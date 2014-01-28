package transport

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"github.com/metcalf/raft"
	"io"
	"log"
	"net"
)

type BinaryTransporter struct {
	listener net.Listener
	server   raft.Server
}

func NewBinaryTransporter() *BinaryTransporter {
	return &BinaryTransporter{}
}

func (t *BinaryTransporter) ListenAndServe(listener net.Listener, server raft.Server) error {
	t.server = server
	t.listener = listener

	var reqType byte

	for {
		conn, err := listener.Accept()
		if err != nil {
			return err
		}

		if reqType, err = t.readPrefix(conn); err != nil {
			return err
		}

		reqData := new(bytes.Buffer)
		t.readData(conn, reqData)

		switch reqType {
		case '\x01':
			err = t.handleVoteRequest(server, reqData, conn)
		case '\x02':
			err = t.handleAppendEntriesRequest(server, reqData, conn)
		case '\x03':
			err = t.handleSnapshotRequest(server, reqData, conn)
		case '\x04':
			err = t.handleSnapshotRecoveryRequest(server, reqData, conn)
		default:
			return fmt.Errorf("Received an invalid request type: %c", reqType)
		}

		conn.Close()

		if err != nil {
			return err
		}
	}

	return nil
}

func (t *BinaryTransporter) SendVoteRequest(server raft.Server, peer *raft.Peer, req *raft.RequestVoteRequest) *raft.RequestVoteResponse {
	resp := &raft.RequestVoteResponse{}

	if !t.sendRequest('\x01', peer, req.Encode, resp.Decode) {
		return nil
	}

	return resp
}

func (t *BinaryTransporter) SendAppendEntriesRequest(server raft.Server, peer *raft.Peer, req *raft.AppendEntriesRequest) *raft.AppendEntriesResponse {
	resp := &raft.AppendEntriesResponse{}

	if !t.sendRequest('\x02', peer, req.Encode, resp.Decode) {
		return nil
	}

	return resp
}

func (t *BinaryTransporter) SendSnapshotRequest(server raft.Server, peer *raft.Peer, req *raft.SnapshotRequest) *raft.SnapshotResponse {
	resp := &raft.SnapshotResponse{}

	if !t.sendRequest('\x03', peer, req.Encode, resp.Decode) {
		return nil
	}

	return resp
}

func (t *BinaryTransporter) SendSnapshotRecoveryRequest(server raft.Server, peer *raft.Peer, req *raft.SnapshotRecoveryRequest) *raft.SnapshotRecoveryResponse {
	resp := &raft.SnapshotRecoveryResponse{}

	if !t.sendRequest('\x04', peer, req.Encode, resp.Decode) {
		return nil
	}

	return resp
}

func (t *BinaryTransporter) sendRequest(reqType byte, peer *raft.Peer, encode func(w io.Writer) (int, error), decode func(r io.Reader) (int, error)) bool {
	var reqData bytes.Buffer

	respData := new(bytes.Buffer)

	if _, err := encode(&reqData); err != nil {
		log.Printf("Encoding error:", err)
		return false
	}

	conn, err := UnixDialer("", peer.ConnectionString)
	if err != nil {
		log.Printf("Could not open connection: %s", err)
		return false
	}
	defer conn.Close()

	if err := t.writeData(conn, reqData.Bytes(), reqType); err != nil {
		log.Printf("Error writing data: %s", err)
		return false
	}

	resType, err := t.readPrefix(conn)
	if err != nil {
		log.Printf("Error reading prefix: %s", err)
		return false
	}
	if resType != reqType {
		log.Printf("Received response of type %c to request of type %c", resType, reqType)
		return false
	}

	if err := t.readData(conn, respData); err != nil {
		log.Printf("Error reading response data: %s", err)
		return false
	}

	if _, err = decode(respData); err != nil && err != io.EOF {
		log.Printf("Decoding error:", err)
		return false
	}

	return true
}

func (t *BinaryTransporter) handleVoteRequest(server raft.Server, r io.Reader, w io.Writer) error {
	respData := new(bytes.Buffer)

	req := &raft.RequestVoteRequest{}

	if _, err := req.Decode(r); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	resp := server.RequestVote(req)
	if _, err := resp.Encode(respData); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	return t.writeData(w, respData.Bytes(), '\x01')
}

func (t *BinaryTransporter) handleAppendEntriesRequest(server raft.Server, r io.Reader, w io.Writer) error {
	respData := new(bytes.Buffer)

	req := &raft.AppendEntriesRequest{}

	if _, err := req.Decode(r); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	resp := server.AppendEntries(req)
	if _, err := resp.Encode(respData); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	return t.writeData(w, respData.Bytes(), '\x02')
}

func (t *BinaryTransporter) handleSnapshotRequest(server raft.Server, r io.Reader, w io.Writer) error {
	respData := new(bytes.Buffer)

	req := &raft.SnapshotRequest{}

	if _, err := req.Decode(r); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	resp := server.RequestSnapshot(req)
	if _, err := resp.Encode(respData); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	return t.writeData(w, respData.Bytes(), '\x03')
}

func (t *BinaryTransporter) handleSnapshotRecoveryRequest(server raft.Server, r io.Reader, w io.Writer) error {
	respData := new(bytes.Buffer)

	req := &raft.SnapshotRecoveryRequest{}

	if _, err := req.Decode(r); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	resp := server.SnapshotRecoveryRequest(req)
	if _, err := resp.Encode(respData); err != nil {
		if wErr := t.writeData(w, []byte{}, '\x00'); wErr != nil {
			return wErr
		}
		return err
	}

	return t.writeData(w, respData.Bytes(), '\x04')
}

func (t *BinaryTransporter) readPrefix(r io.Reader) (byte, error) {
	var prefix byte
	var reqType byte

	if err := binary.Read(r, binary.BigEndian, &prefix); err != nil {
		return '\x00', err
	}
	if prefix != '\xfe' {
		return '\x00', fmt.Errorf("Prefix bit was unexpectedly: %c", prefix)
	}

	if err := binary.Read(r, binary.BigEndian, &reqType); err != nil {
		return '\x00', err
	}

	return reqType, nil
}

func (t *BinaryTransporter) readData(r io.Reader, w io.Writer) error {
	var respLen uint32

	if err := binary.Read(r, binary.BigEndian, &respLen); err != nil {
		return err
	}

	if _, err := io.CopyN(w, r, int64(respLen)); err != nil {
		return err
	}

	return nil
}

func (t *BinaryTransporter) writeData(w io.Writer, data []byte, reqType byte) error {
	preamble := new(bytes.Buffer)

	if err := binary.Write(preamble, binary.BigEndian, '\xfe'); err != nil {
		return err
	}
	if err := binary.Write(preamble, binary.BigEndian, reqType); err != nil {
		return err
	}
	if err := binary.Write(preamble, binary.BigEndian, uint32(len(data))); err != nil {
		return err
	}

	if _, err := w.Write(preamble.Bytes()); err != nil {
		return err
	}
	if _, err := w.Write(data); err != nil {
		return err
	}

	return nil
}
