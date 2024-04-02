package bluetooth

import (
	"log"
	"time"
    "bytes"

	"go.bug.st/serial"
)

const(
    ACK = 0x69
    REGISTER = 0x42

    DDMMYYYY = "02-01-2006"
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
            log.Printf("Read buf as %x", buf[0])
            if buf[0] == REGISTER {
                time.Sleep(2*time.Second)
                packet := make([]byte, 26);
                nbytes, err := port.Read(packet);
                if err != nil || nbytes != 26 {
                    log.Println("Error in reading the packet")
                }
                rollno, phoneno, adminpass, date := parseRegisterPacket(packet)
                log.Printf("%s %s %s %s\n", rollno, phoneno, adminpass, date)

            }
            time.Sleep(1*time.Second)
        }
    }()
}


func parseRegisterPacket(packet []byte) (string, string, string, string){
    rollbytes := bytes.NewBufferString("")
    phonebytes := bytes.NewBufferString("")
    adminbytes := bytes.NewBufferString("")

    for i := 0; i < 6; i++ {
        rollbytes.WriteByte(packet[i])
    }
    for i := 6; i < 16; i++ {
        phonebytes.WriteByte(packet[i])
    }
    for i := 16; i < 26; i++ {
        adminbytes.WriteByte(packet[i])
    }
    
    now := time.Now().UTC()
    
    return rollbytes.String(), phonebytes.String(), adminbytes.String(), now.Format(DDMMYYYY)
}

