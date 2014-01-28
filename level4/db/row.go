package db

import (
	"fmt"
)

type Row struct {
	name         string
	friendCount  uint32
	requestCount uint32
	favoriteWord string
}

func (r *Row) Format() string {
	return fmt.Sprintf("%s|%d|%d|%s",
		r.name,
		r.friendCount,
		r.requestCount,
		r.favoriteWord)
}

func (r *Row) RequestCount() uint32 {
	return r.requestCount
}
