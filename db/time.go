package db

import (
	"encoding/binary"
	"time"
)

const SecondsBitOffset = 20

// shiftTime converts Go time into a Sky timestamp.
func shiftTime(value time.Time) int64 {
	timestamp := value.UnixNano() / 1000
	usec := timestamp % 1000000
	sec := timestamp / 1000000
	return (sec << SecondsBitOffset) + usec
}

// shiftTimeBytes converts Go time into a byte slice in Sky timestamp format.
func shiftTimeBytes(value time.Time) []byte {
	var b [8]byte
	bs := b[:8]
	timestamp := shiftTime(value)
	binary.BigEndian.PutUint64(bs, uint64(timestamp))
	return bs
}

// unshiftTime converts a Sky timestamp into Go time.
func unshiftTime(value int64) time.Time {
	usec := value & 0xFFFFF
	sec := value >> SecondsBitOffset
	return time.Unix(sec, usec*1000).UTC()
}

// unshiftTimeBytes converts a byte slice containing a Sky timestamp to Go time.
func unshiftTimeBytes(value []byte) time.Time {
	return unshiftTime(int64(binary.BigEndian.Uint64(value)))
}
