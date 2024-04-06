package main

import (
	"log"
	//"time"

	"github.com/ShivamIITK21/Biometric-Attendance/server/bluetooth"
	//"go.bug.st/serial"
)

func main(){
     b := bluetooth.New("/dev/rfcomm0")
     log.Println("Starting Listener....")
     b.Run()
     for{}
    // mode := &serial.Mode{
    //     BaudRate: 9600,
    // }
    // port, err := serial.Open("/dev/rfcomm0",mode)
    // if err != nil {
    //     log.Fatal(err)
    // }
    // buf := make([]byte, 1)
    // for{
    //     _, err = port.Read(buf);
    //     if err != nil {
    //         log.Println(err)
    //     }
    //     log.Printf("Echo Back %d", buf[0])
    //     port.Write(buf);
    //     time.Sleep(1*time.Second)
    // }
}
