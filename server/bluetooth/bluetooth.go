package bluetooth

import (
	"log"
	"os"
    "errors"
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
    f, err := os.Open(b.filepath) 
    defer f.Close()
    if err != nil{
        return err
    }
    Listen(f)
    return nil
}

func Listen(f *os.File){
    buf := make([]byte, 1) 
    go func(){
        for{
            bytesRead, err := f.Read(buf)
            if err != nil {
                log.Println("Error in reading bytes")
                continue
            }
            if bytesRead != 1 {
                log.Printf("Expected to read 1 byte, read %d\n", bytesRead)
                continue
            }
            if buf[0] == STORE {
                err := StoreHandler(f)
                if err != nil {
                    log.Println("Error in Storing")
                }
            }
        }
    }()
}

func StoreHandler(f *os.File) error {
    buf := make([]byte, 512)
    bytesRead, err := f.Read(buf)
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



