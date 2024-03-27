package main

import (
	"log"

	"github.com/ShivamIITK21/Biometric-Attendance/server/bluetooth"
)

func main(){
    b := bluetooth.New("/dev/rfcomm0")
    log.Println("Starting Listener....")
    b.Run()
    for{}
}
