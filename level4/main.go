package main

import (
	"flag"
	"fmt"
	"github.com/narced133/ctf3/level4/cluster"
	"github.com/narced133/ctf3/level4/debuglog"
	"github.com/narced133/ctf3/level4/server"
	"log"
	"os"
	"path/filepath"
)

func main() {
	var verbose bool
	var listen, join, directory string
	var err error

	flag.BoolVar(&verbose, "v", false, "Enable debug output")
	flag.StringVar(&listen, "l", "127.0.0.1:4000", "Socket to listen on (Unix or TCP)")
	flag.StringVar(&join, "join", "", "Cluster to join")
	flag.StringVar(&directory, "d", "/tmp/sqlcluster", "Storage directory")

	dir := filepath.Dir(os.Args[0])
	base := "./" + filepath.Base(os.Args[0])
	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, `Usage: %s [options]

Run a highly-available SQLite.

By default, SQLCluster will listen on a TCP port. However, if you
specify a listen address that begins with a / or ., that will be
interpeted as Unix path for SQLCluster to listen on.

So for example, you could run:

  %s -d /tmp/sqlcluster/node0 -l /tmp/server.sock

And SQLCluster would listen at /tmp/server.sock.

(Note that Octopus will run using Unix sockets only, but it will
probably be more convient for you to develop using TCP.)

Run a cluster as follows:

  cd %s
  %s -d /tmp/sqlcluster/node0 &
  %s -d /tmp/sqlcluster/node1 -l 127.0.0.1:4001 --join 127.0.0.1:4000 &
  %s -d /tmp/sqlcluster/node2 -l 127.0.0.1:4002 --join 127.0.0.1:4000

OPTIONS:
`, os.Args[0], base, dir, base, base, base)
		flag.PrintDefaults()
	}

	flag.Parse()

	if flag.NArg() != 0 {
		flag.Usage()
		os.Exit(1)
	}

	debuglog.SetVerbose(verbose)

	if err := os.MkdirAll(directory, os.ModeDir|0755); err != nil {
		log.Fatalf("Error while creating storage directory: %s\n", err)
	}

	if err := os.Chdir(directory); err != nil {
		log.Fatalf("Error while changing to storage directory: %s\n", err)
	}

	log.Printf("Changing directory to %s", directory)

	s, err := server.New()
	if err != nil {
		log.Fatal(err)
	}

	c, err := cluster.New(directory, listen, s.ListenAndServe)
	if err != nil {
		log.Fatal(err)
	}

	if err := c.ListenAndServe(join); err != nil {
		log.Fatal(err)
	}
}
