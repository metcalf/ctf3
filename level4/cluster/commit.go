package cluster

type Commit struct {
	entries []byte
	error   error
}

func (c *Commit) Error() error {
	return c.error
}

func (c *Commit) Entries() [][]byte {
	return c.entries
}

func (c *Commit) Index() int {
	return len(c.entries)
}
