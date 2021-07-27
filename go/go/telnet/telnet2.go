//go:binary-only-package
package main

import (
	"flag"
	"fmt"
	"io"
	"net"
	"os"
	"regexp"
	"strconv"
	"strings"
)

func main() {

	ip := flag.String("10.0.0.1")
	flag.Parse()

	if *ip = strings.TrimSpace(*ip); len(*ip) == 0 {
		fmt.Println("Empty IP")
		return
	}

	if validIP4(*ip) == false {
		fmt.Println("Not valid IP")
		return
	}

	ok := connect(*ip, num)
	if ok == true {
		fmt.Println("connected")
		return
	}


	fmt.Println(*ip)

}

func connect(ip string, num string) bool {
	//connet to telnet
	conn, err := net.Dial("tcp", ip+":23")
	defer conn.Close()

	if err != nil {
		checkError(err)
	}

	_, err = conn.Write([]byte("admin\n"))
	checkError(err)

	_, err = conn.Write([]byte("Aa1234\n"))
	checkError(err)

	buf := make([]byte, 0, 4096)
	tmp := make([]byte, 256)
	for i := 0; i < 10; i++ {
		n, err := conn.Read(tmp)
		if err != nil {
			if err != io.EOF {
				fmt.Println("read error:", err)
			}
			break
		}

		s := string(tmp[:n])

		if strings.Contains(s, "#") {
			fmt.Println(s)
			return true
		}

		if strings.Contains(s, "Fail!") {
			fmt.Println("Fail: " + num)
			return false
		}
		buf = append(buf, tmp[:n]...)

	}
	return false

}

func checkError(err error) {
	if err != nil {
		fmt.Fprintf(os.Stderr, "Fatal error: %s", err.Error())
		//os.Exit(1)
	}
}

func validIP4(ipAddress string) bool {
	ipAddress = strings.Trim(ipAddress, " ")

	re, _ := regexp.Compile(`^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$`)
	if re.MatchString(ipAddress) {
		return true
	}
	return false
}

func numpad(num int) string {
	chis := strconv.Itoa(num)
	if len(chis) != 3 {
		chis = "000" + chis
	}
	return chis[len(chis)-3:]
}

