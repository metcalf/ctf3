package debuglog

import (
	"log"
	"os"
)

type Logger struct {
	*log.Logger
	verbose bool
}

func New() *Logger {
	return &Logger{
		log.New(os.Stderr, "", log.LstdFlags),
		false,
	}
}

var std = New()

// debug logging methods:

func Verbose() bool {
	return std.verbose
}

func SetVerbose(verbose bool) {
	std.verbose = verbose
}

func Debugln(v ...interface{}) {
	if std.verbose {
		std.Println(v...)
	}
}

func Debugf(format string, v ...interface{}) {
	if std.verbose {
		std.Printf(format, v...)
	}
}
