package bluetooth

import (
	"bytes"
	"log"
	"time"

	"github.com/ShivamIITK21/Biometric-Attendance/server/db"
	"github.com/ShivamIITK21/Biometric-Attendance/server/models"
	"go.bug.st/serial"
)

const(
    ACK = 0x69
    REGISTER = 0x42
    ATTENDANCE = 0x96

	DDMMYY = "02-01-06"
)

type Bluetooth struct {
	filepath string
}

func New(devFile string) *Bluetooth {
	b := Bluetooth{filepath: devFile}
	return &b
}

func (b *Bluetooth) Run() error {
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
                time.Sleep(5*time.Second)
                packet := make([]byte, 27);
                nbytes, err := port.Read(packet);
                if err != nil {
                    log.Println("Error in reading the packet")
                }else if nbytes != 27{
                    log.Println("Bytes received less than 27")
                }
                rollno, phoneno, adminpass, date, id := parseRegisterPacket(packet)
                log.Printf("Register packet req with %s %s %s %s %d\n", rollno, phoneno, adminpass, date, id)
                var admin models.Admin
                result := db.DB.First(&admin)
                if result.Error != nil {
                    log.Println("No admin registered")
                }
                if (admin.Password == adminpass) {
                    writeRegisterResponsePacket(port, true, date)
                    newuser := models.User{RollNo: rollno, PhoneNo: phoneno, CreatedOn: date, ID: uint(id)}
                    db.DB.Create(&newuser)
                } else {
                    writeRegisterResponsePacket(port, false, date)
                }
            } else if buf[0] == ATTENDANCE {
                time.Sleep(2*time.Second)
                packet := make([]byte, 1)
                nbytes, err := port.Read(packet);
                if err != nil {
                    log.Println("Error in reading the packet")
                }else if nbytes != 1{
                    log.Println("Bytes received less than 1")
                }
                id_matched := parseAttendancePacket(packet)
                log.Printf("Req with %d", id_matched)
                var user models.User
                res := db.DB.First(&user, id_matched)
                if res.Error != nil {
                    log.Println("Error in getting user info")
                }
                log.Println("Found User->")
                log.Print(user)
                date := time.Now().UTC().Format(DDMMYY)
                writeAttendanceResponsePacket(port, date, user.RollNo, user.PhoneNo)
            }
            time.Sleep(1*time.Second)
        }
    }()
}

func parseAttendancePacket(packet []byte) uint{
    return uint(packet[0])
}

func parseRegisterPacket(packet []byte) (string, string, string, string, byte ){
    rollbytes := bytes.NewBufferString("")
    phonebytes := bytes.NewBufferString("")
    adminbytes := bytes.NewBufferString("")

    for i := 0; i < 6; i++ {
        if packet[i] == 'x' {
            break
        }
        rollbytes.WriteByte(packet[i])
    }
    for i := 6; i < 16; i++ {
        if packet[i] == 'x' {
            break
        }
        phonebytes.WriteByte(packet[i])
    }
    for i := 16; i < 26; i++ {
        if packet[i] == 'x' {
            break
        }
        adminbytes.WriteByte(packet[i])
    }
    
    id := packet[26]
    
    now := time.Now().UTC()
    
    return rollbytes.String(), phonebytes.String(), adminbytes.String(), now.Format(DDMMYY), id
}

func writeRegisterResponsePacket(port serial.Port, isAdmin bool, date string){
    response := make([]byte, 10)
    response[0] = ACK
    if isAdmin {
        response[1] = 1
    } else{
        response[1] = 47 
    } 
    for i := 0; i < 8; i++ {
        var b byte
        b = date[i]
        response[i+2] = b
    }
    log.Println("Writing....")
    bytesw, err := port.Write(response)
    if bytesw != 10 || err != nil {
        log.Println("Error in writing register response packet")
    }
}

func writeAttendanceResponsePacket(port serial.Port, date string, rollno string, phno string) {
    response := make([]byte, 25)
    response[0] = ACK
    idx := 1 
    for i := 0; i < 8; i++{
        if i < len(date){
            response[idx] = byte(date[i])
        }else{
            response[idx] = 'x'
        }
        idx++
    }
    for i := 0; i < 6; i++{
        if i < len(rollno){
            response[idx] = byte(rollno[i])
        }else{
            response[idx] = 'x'
        }
        idx++
    }
    for i := 0; i < 10; i++{
        if i < len(phno){
            response[idx] = byte(phno[i])
        }else{
            response[idx] = 'x'
        }
        idx++
    }
    log.Println("Writing....")
    bytesw, err := port.Write(response)
    if bytesw != 25 || err != nil {
        log.Println("Error in writing register response packet")
    }
}

