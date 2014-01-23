package transport

import (
	"log"
	"net"
)

type AcceptResponse struct {
	conn net.Conn
	err  error
}

type RaftListener struct {
	listener       net.Listener
	clusterChannel chan *AcceptResponse
	requestChannel chan *AcceptResponse
	accepting      bool
}

func (m *RaftListener) ClusterListener() net.Listener {
	return &ListenerProxy{
		listener: m.listener,
		accept: func() (c net.Conn, err error) {
			m.StartAccepting()
			resp := <-m.clusterChannel
			return resp.conn, resp.err
		},
	}
}

func (m *RaftListener) RequestListener() net.Listener {
	return &ListenerProxy{
		listener: m.listener,
		accept: func() (c net.Conn, err error) {
			m.StartAccepting()
			resp := <-m.requestChannel
			return resp.conn, resp.err
		},
	}
}

func (m *RaftListener) StartAccepting() error {
	if m.accepting {
		return nil
	}

	m.requestChannel = make(chan *AcceptResponse)
	m.clusterChannel = make(chan *AcceptResponse)

	go func() {
		var chunk []byte

		for {
			conn, err := m.listener.Accept()

			resp := &AcceptResponse{}

			if err != nil {
				resp.err = err
				m.requestChannel <- resp
				m.clusterChannel <- resp
				break
			}

			n, err := conn.Read(chunk)

			if n < 1 {
				log.Fatal("Connection read returned nothing")
			}
			if err != nil {
				resp.err = err

				m.requestChannel <- resp
				m.clusterChannel <- resp
				break
			}

			resp.conn = &ConnectionProxy{
				conn:           conn,
				firstChunk:     chunk,
				firstChunkRead: false,
			}

			if chunk[0] == '\xfe' {
				m.requestChannel <- resp
			} else {
				m.requestChannel <- resp
			}
		}
	}()

	return nil
}

func Listen(addr string) (net.Listener, error) {
	network := Network(addr)
	log.Printf("Listening on %s: %s", network, addr)

	listener, err := net.Listen(network, addr)
	if err != nil {
		return nil, err
	}

	return listener, nil
	/*multiListener := &RaftListener{
		listener: listener,
	}

	return multiListener, nil*/
}
