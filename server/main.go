package main

import (
	"log"
	//"time"

    "github.com/ShivamIITK21/Biometric-Attendance/server/api"
	"github.com/ShivamIITK21/Biometric-Attendance/server/bluetooth"
	//"go.bug.st/serial"
)

// type Server struct {
//     b *bluetooth.Bluetooth
//     api *api.API
// }

func main(){

    // go api.StartAPI()
    data := make(chan []byte)
    b := bluetooth.New("/dev/rfcomm0", data)
    
    // b := bluetooth.New("/dev/rfcomm0", data)
    log.Println("Starting Listener....")
    api:= api.New(data)
    go api.StartAPI()
    go b.Run()

    // b.Run()
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
