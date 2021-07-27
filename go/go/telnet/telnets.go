package main

import (
	"github.com/reiver/go-telnet"
	"crypto/tls"
)

func main() {
	tlsConfig := &tls.Config{}
	var caller telnet.Caller = telnet.StandardCaller

	//@TOOD: replace "example.net:5555" with address you want to connect to.
	telnet.DialToAndCallTLS("10.0.0.1:23", caller, tlsConfig)
}
