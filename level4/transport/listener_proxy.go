package transport

import (
	"net"
)

type ListenerProxy struct {
	listener net.Listener
	accept   func() (c net.Conn, err error)
}

func (l *ListenerProxy) Accept() (c net.Conn, err error) {
	return l.accept()
}

func (l *ListenerProxy) Close() error {
	return l.listener.Close()
}

func (l *ListenerProxy) Addr() net.Addr {
	return l.listener.Addr()
}
