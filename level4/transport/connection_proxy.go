package transport

import (
	"net"
	"time"
)

type ConnectionProxy struct {
	conn           net.Conn
	firstChunk     []byte
	firstChunkRead bool
}

func (c *ConnectionProxy) Read(b []byte) (n int, err error) {
	if c.firstChunkRead {
		return c.conn.Read(b)
	} else {
		copy(b, c.firstChunk)
		return len(c.firstChunk), nil
	}
}

func (c *ConnectionProxy) Write(b []byte) (n int, err error) {
	return c.conn.Write(b)
}

func (c *ConnectionProxy) Close() error {
	return c.conn.Close()
}

func (c *ConnectionProxy) LocalAddr() net.Addr {
	return c.conn.LocalAddr()
}

func (c *ConnectionProxy) RemoteAddr() net.Addr {
	return c.conn.RemoteAddr()
}

func (c *ConnectionProxy) SetDeadline(t time.Time) error {
	return c.conn.SetDeadline(t)
}

func (c *ConnectionProxy) SetReadDeadline(t time.Time) error {
	return c.conn.SetReadDeadline(t)
}

func (c *ConnectionProxy) SetWriteDeadline(t time.Time) error {
	return c.conn.SetWriteDeadline(t)
}
