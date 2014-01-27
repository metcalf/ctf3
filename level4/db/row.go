package db

import (
	"fmt"
)

type Row struct {
	name         string
	friendCount  uint8
	requestCount uint16
	favoriteWord string
}

func (r *Row) Format() string {
	return fmt.Sprintf("%s|%d|%d|%s",
		r.name,
		r.friendCount,
		r.requestCount,
		r.favoriteWord)
}

func (r *Row) RequestCount() uint16 {
	return r.requestCount
}
