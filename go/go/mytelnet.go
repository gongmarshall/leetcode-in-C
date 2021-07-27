package main

import (
	"fmt"
	"infrastructure/log"
	"net"
	"strconv"
	"strings"
	"time"
)

type TelnetClient struct {
	IP               string
	Port             string
	IsAuthentication bool
	UserName         string
	Password         string
}

const (
	//经过测试，linux下，延时需要大于100ms
	TIME_DELAY_AFTER_WRITE = 500 //500ms
)

func main() {
	telnetClientObj := new(TelnetClient)
	telnetClientObj.IP = "10.0.0.1"
	telnetClientObj.Port = "23"
	telnetClientObj.IsAuthentication = true
	telnetClientObj.UserName = "admin"
	telnetClientObj.Password = "Aa1234"
	//	fmt.Println(telnetClientObj.PortIsOpen(5))
	action := []string{"w_cd /", "r_1024", "w_ls", "r_1024"}
	telnetClientObj.Telnet(action, 20)
}

func (this *TelnetClient) PortIsOpen(timeout int) bool {
	raddr := this.IP + ":" + this.Port
	conn, err := net.DialTimeout("tcp", raddr, time.Duration(timeout)*time.Second)
	if nil != err {
		log.Error("pkg: model, func: PortIsOpen, method: net.DialTimeout, errInfo:", err)
		return false
	}
	defer conn.Close()
	return true
}

func (this *TelnetClient) Telnet(action []string, timeout int) (buf []byte, err error) {
	raddr := this.IP + ":" + this.Port
	conn, err := net.DialTimeout("tcp", raddr, time.Duration(timeout)*time.Second)
	if nil != err {
		log.Error("pkg: model, func: Telnet, method: net.DialTimeout, errInfo:", err)
		return
	}
	defer conn.Close()
	if false == this.telnetProtocolHandshake(conn) {
		log.Error("pkg: model, func: Telnet, method: this.telnetProtocolHandshake, errInfo: telnet protocol handshake failed!!!")
		return
	}
	//	conn.SetReadDeadline(time.Now().Add(time.Second * 30))
	for _, v := range action {
		actSlice := strings.SplitN(v, "_", 2)
		if 2 > len(actSlice) {
			log.Error("pkg: model, func: Telnet, method: strings.SplitN, errInfo: Invalid command\n", v)
			return
		}
		switch actSlice[0] {
		case "r":
			var n int
			n, err = strconv.Atoi(actSlice[1])
			if nil != err {
				log.Error("pkg: model, func: Telnet, method: strconv.Atoi, errInfo:", err)
				return
			}
			p := make([]byte, n)
			//	p := make([]byte, 0, n)
			n, err = conn.Read(p[0:])
			if nil != err {
				log.Error("pkg: model, func: Telnet, method: conn.Read, errInfo:", err)
				return
			}
			buf = append(buf, p[0:n]...)
			fmt.Println("read data length:", n)
			fmt.Println(string(p[0:n]) + "\n\n")
			//	fmt.Println(buf)
		case "w":
			_, err = conn.Write([]byte(actSlice[1] + "\n"))
			if nil != err {
				log.Error("pkg: model, func: Telnet, method: conn.Write, errInfo:", err)
				return
			}
			//	fmt.Println("wirte:", actSlice[1])
			time.Sleep(time.Millisecond * TIME_DELAY_AFTER_WRITE)
		}
	}
	return
}

func (this *TelnetClient) telnetProtocolHandshake(conn net.Conn) bool {
	var buf [4096]byte
	n, err := conn.Read(buf[0:])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Read, errInfo:", err)
		return false
	}
	fmt.Println(string(buf[0:n]))
	fmt.Println((buf[0:n]))

	buf[1] = 252
	buf[4] = 252
	buf[7] = 252
	buf[10] = 252
	fmt.Println((buf[0:n]))
	n, err = conn.Write(buf[0:n])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Write, errInfo:", err)
		return false
	}

	n, err = conn.Read(buf[0:])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Read, errInfo:", err)
		return false
	}
	fmt.Println(string(buf[0:n]))
	fmt.Println((buf[0:n]))

	buf[1] = 252
	buf[4] = 251
	buf[7] = 252
	buf[10] = 254
	buf[13] = 252
	fmt.Println((buf[0:n]))
	n, err = conn.Write(buf[0:n])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Write, errInfo:", err)
		return false
	}

	n, err = conn.Read(buf[0:])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Read, errInfo:", err)
		return false
	}
	fmt.Println(string(buf[0:n]))
	fmt.Println((buf[0:n]))

	buf[1] = 252
	buf[4] = 252
	fmt.Println((buf[0:n]))
	n, err = conn.Write(buf[0:n])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Write, errInfo:", err)
		return false
	}

	n, err = conn.Read(buf[0:])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Read, errInfo:", err)
		return false
	}
	fmt.Println(string(buf[0:n]))
	fmt.Println((buf[0:n]))

	if false == this.IsAuthentication {
		return true
	}

	n, err = conn.Write([]byte(this.UserName + "\n"))
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Write, errInfo:", err)
		return false
	}
	time.Sleep(time.Millisecond * TIME_DELAY_AFTER_WRITE)

	n, err = conn.Read(buf[0:])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Read, errInfo:", err)
		return false
	}
	fmt.Println(string(buf[0:n]))

	n, err = conn.Write([]byte(this.Password + "\n"))
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Write, errInfo:", err)
		return false
	}
	time.Sleep(time.Millisecond * TIME_DELAY_AFTER_WRITE)

	n, err = conn.Read(buf[0:])
	if nil != err {
		log.Error("pkg: model, func: telnetProtocolHandshake, method: conn.Read, errInfo:", err)
		return false
	}
	fmt.Println(string(buf[0:n]))
	return true
}
