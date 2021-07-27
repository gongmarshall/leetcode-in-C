package main

import (
	"github.com/reiver/go-telnet"
	"time"
)

func main() {
//	var caller telnet.Caller = telnet.StandardCaller

	//@TOOD: replace "example.net:5555" with address you want to connect to.
//	telnet.DialToAndCall("10.0.0.1:23", caller)
	conn, _ := telnet.DialTo("10.0.0.1:23")
	time.Sleep(time.Duration(1)*time.Second)
	conn.Write([]byte("admin"))
	conn.Write([]byte("\n"))
	time.Sleep(time.Duration(1)*time.Second)
	conn.Write([]byte("Aa1234"))
	conn.Write([]byte("\n"))
	time.Sleep(time.Duration(1)*time.Second)
	conn.Write([]byte("echo 1234 >/tmp/telnet_test.txt"))
	conn.Write([]byte("\n"))
	time.Sleep(time.Duration(1)*time.Second)

}

