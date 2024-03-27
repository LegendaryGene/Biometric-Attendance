package bluetooth

import (
	"errors"
	"log"
	"time"

	"go.bug.st/serial"
)

const(
    STORE = 0x67
    GET   = 0x68
    ACK   = 0x69
)

type Bluetooth struct {
    filepath    string
}

func New(devFile string) *Bluetooth{
    b := Bluetooth{filepath: devFile}
    return &b
}

func(b *Bluetooth) Run() error{
    mode := &serial.Mode{
        BaudRate: 9600,
    }
    port, err := serial.Open(b.filepath, mode)
    if err != nil {
        log.Fatal(err)
    }
    Listen(port)
    return nil
}

func Listen(port serial.Port){
    buf := make([]byte, 1) 
    go func(){
        for{
            bytesRead, err := port.Read(buf)
            if err != nil {
                log.Println(err)
                continue
            }
            if bytesRead != 1 {
                log.Printf("Expected to read 1 byte, read %d\n", bytesRead)
                continue
            }
            if buf[0] == STORE {
                err := StoreHandler(port)
                if err != nil {
                    log.Println(err)
                }
                buf[0] = ACK
                n, err := port.Write(buf);
                if err != nil {
                    log.Println(err)
                }
                log.Printf("Wrote %d \n", n) 
            }
            time.Sleep(1*time.Second)
        }
    }()
}

func StoreHandler(port serial.Port) error {
    time.Sleep(5*time.Second)
    buf := make([]byte, 512)
    bytesRead, err := port.Read(buf)
    log.Printf("read %d bytes", bytesRead)
    if err != nil {
        return err
    }
    if bytesRead != 512 {
        return errors.New("Expected 512 bytes") 
    }
    log.Println("Recv request to store template")
    log.Print(buf)
    return nil
}



