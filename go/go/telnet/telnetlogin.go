package main

import(
	"fmt"
	"os"
	"time"
	"os/exec"
)

func main() {
	cmd := exec.Command("/home/marshall/study/go/telnet/mytelent")
	data, err := cmd.Output()
	if err != nil {
		fmt.Println("failed to call cmd.Run(): %v", err)
	}
	fmt.Fprintln(os.Stdin, "admin\n")
	time.Sleep(time.Duration(1)*time.Second)
	fmt.Fprintln(os.Stdin, "Aa1234\n")
	time.Sleep(time.Duration(1)*time.Second)
	fmt.Fprintln(os.Stdin, "echo abc >/tmp/telnet_test.txt\n")
	fmt.Println("output: %s", string(data))
}
