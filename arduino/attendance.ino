#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

const byte ROWS = 4;
const byte COLS = 4;

const int LCD_RS = A5;
const int LCD_EN = A4;
const int LCD_D4 = A3;
const int LCD_D5 = A2;
const int LCD_D6 = A1;
const int LCD_D7 = A0;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {9, 8, 7, 6};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

SoftwareSerial blueSerial(2, 3);
SoftwareSerial A9Serial(0, 1);
SoftwareSerial FingerSerial(4, 5);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FingerSerial);

char password[11];
char phone[11];
char rollno[7];
uint8_t id = 1;
uint8_t id_detected = 0;
char recvd_phone[10];
char recvd_rollno[6];
char recvd_date[8];
char auth = 0;
// char date[8];

void lcdPrint(String top, String bottom)
{
    lcd.setCursor(0, 0);
    lcd.print(top);
    lcd.setCursor(0, 1);
    lcd.print(bottom);
}

void writeRegisterPacket()
{
    blueSerial.listen();
    char packet[28];
    packet[0] = 0x42;
    int idx = 1;
    for (int i = 0; i < 6; i++)
        packet[idx++] = rollno[i];
    for (int i = 0; i < 10; i++)
        packet[idx++] = phone[i];
    for (int i = 0; i < 10; i++)
        packet[idx++] = password[i];
    packet[27] = id;
    for (int i = 0; i < 28; i++)
        blueSerial.write(packet[i]);
}

void readRegisterResponsePacket()
{
    char res[10];
    uint8_t nRead = 0;
    blueSerial.listen();
    while (nRead != 10)
    {
        if (blueSerial.available() > 0)
        {
            res[nRead++] = blueSerial.read();
        }
    }
    for (int i = 0; i < 10; i++)
        Serial.print(res[i]);
    Serial.println();
    auth = res[1];
    for (int i = 2; i < 10; i++)
        recvd_date[i - 2] = res[i];
}

void clearDisplay()
{
    String clearstring = "                ";
    lcdPrint(clearstring, clearstring);
}

void writeAttendancePacket()
{
    blueSerial.listen();
    char packet[2];
    packet[0] = 0x96;
    packet[1] = id_detected;
    blueSerial.write(packet[0]);
    blueSerial.write(packet[1]);
}

void readAttendanceResponsePacket()
{
    char res[25];
    uint8_t nRead = 0;
    blueSerial.listen();
    while (nRead != 25)
    {
        if (blueSerial.available() > 0)
        {
            res[nRead++] = blueSerial.read();
        }
    }
    for (int i = 0; i < 25; i++)
        Serial.print(res[i]);
    Serial.println();
    for (int i = 1; i < 9; i++)
        recvd_date[i - 2] = res[i];
    for (int i = 9; i < 15; i++)
        recvd_rollno[i - 9] = res[i];
    for (int i = 15; i < 25; i++)
        recvd_phone[i - 15] = res[i];
}

char taskInput()
{
    clearDisplay();
    lcdPrint("A - Register", "B - Mark Attnd");
    Serial.println("Register(A) or Mark Attendance(B)?");
    char inp = customKeypad.waitForKey();
    return inp;
}

void clean(char *buf, int len)
{
    for (int i = 0; i < len; i++)
    {
        buf[i] = '\0';
    }
}

byte takePassword()
{
    clearDisplay();
    clean(password, 10);
    Serial.println("Enter Admin Password");
    String top = "Enter Adm Pwd";
    lcdPrint(top, "");
    byte idx = 0;
    char c = 0;
    while (idx < 10 && c != 'D')
    {
        c = customKeypad.waitForKey();
        if (c == 'D')
        {
            Serial.println();
            break;
        }
        Serial.print(c);
        password[idx] = c;
        lcdPrint(top, password);
        idx++;
    }
    if (idx != 10)
    {
        for (int i = idx; i < 11; i++)
            password[i] = 'x';
    }
    return idx;
}

void takePhone()
{
    Serial.println("Enter Phone Number");
    clearDisplay();
    clean(phone, 10);
    String top = "Enter Phone No.";
    lcdPrint(top, "");
    byte idx = 0;
    char c = 0;
    while (idx < 11 && c != 'D')
    {
        c = customKeypad.waitForKey();
        if (c == 'D')
        {
            Serial.println();
            break;
        }
        Serial.print(c);
        phone[idx] = c;
        lcdPrint(top, phone);
        idx++;
    }
    for (int i = idx; i < 11; i++)
        phone[i] = 'x';
}

void takeRollNo()
{
    Serial.println("Enter Roll Number");
    clearDisplay();
    clean(rollno, 6);
    String top = "Enter Roll No.";
    lcdPrint(top, "");
    byte idx = 0;
    char c = 0;
    while (idx < 7 && c != 'D')
    {
        c = customKeypad.waitForKey();
        if (c == 'D')
        {
            Serial.println();
            break;
        }
        Serial.print(c);
        rollno[idx] = c;
        lcdPrint(top, rollno);
        idx++;
    }
    for (int i = idx; i < 7; i++)
        rollno[i] = 'x';
}

void SendMessage(int mode)
{
    A9Serial.println("AT+CMGF=1"); // Sets the GSM Module in Text Mode
    delay(1000);                   // Delay of 1000 milli seconds or 1 second
    if (mode == 0)
    {
        A9Serial.println(String("AT+CMGS=\"+91") + phone + String("\"\r")); // Replace x with mobile number
        delay(1000);
        A9Serial.println(String("Roll Number ") + rollno + String(" has been registered on ") + recvd_date); // The SMS text you want to send
        delay(100);
        A9Serial.println((char)26); // ASCII code of CTRL+Z
        delay(1000);
    }
    else
    {
        A9Serial.println(String("AT+CMGS=\"+91") + recvd_phone + String("\"\r")); // Replace x with mobile number
        delay(1000);
        A9Serial.println(String("Roll Number ") + recvd_rollno + String(" has been marked present for ") + recvd_date); // The SMS text you want to send
        delay(100);
        A9Serial.println((char)26); // ASCII code of CTRL+Z
        delay(1000);
    }
}

uint8_t getFingerprintEnroll()
{

    FingerSerial.listen();
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");
    Serial.println(id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(id);
    p = -1;
    Serial.println("Place same finger again");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    Serial.print("Creating model for #");
    Serial.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println("Fingerprints did not match");
        return false;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    return true;
}

bool storeTemplate()
{
    FingerSerial.listen();
    int p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Stored!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return false;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println("Could not store in that location");
        return false;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println("Error writing to flash");
        return false;
    }
    else
    {
        Serial.println("Unknown error");
        return false;
    }
    id++;
    return true;
}

void setup()
{
    Serial.begin(9600);
    blueSerial.begin(9600);
    A9Serial.begin(115200);
    lcd.begin(16, 2);
    //    String out= "top";
    //    String down = "down";
    //    char cc[3];
    //    cc[0]='A';cc[1]='B';cc[2]='\0';
    //    lcdPrint(out, cc);
    finger.begin(57600);
    if (finger.verifyPassword())
    {
        Serial.println("Found fingerprint sensor!");
    }
    else
    {
        Serial.println("Did not find fingerprint sensor :(");
        while (1)
        {
            delay(1);
        }
    }

    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x"));
    Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));
    Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: "));
    Serial.println(finger.capacity);
    Serial.print(F("Security level: "));
    Serial.println(finger.security_level);
    Serial.print(F("Device address: "));
    Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));
    Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: "));
    Serial.println(finger.baud_rate);
}

bool getFingerprintID()
{
    FingerSerial.listen();
    delay(3000);
    uint8_t p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
    case FINGERPRINT_NOFINGER:
        Serial.println("No finger detected");
        return false;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK)
    {
        Serial.println("Found a print match!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println("Did not find a match");
        return p;
    }
    else
    {
        Serial.println("Unknown error");
        return p;
    }

    // found a match!
    Serial.print("Found ID #");
    Serial.print(finger.fingerID);
    Serial.print(" with confidence of ");
    Serial.println(finger.confidence);

    id_detected = finger.fingerID;
    return true;
}

void loop()
{
    char inp = taskInput();
    Serial.println(inp);
    if (inp == 'A')
    {
        Serial.println("Registration Request Initiated");
        byte len = takePassword();
        takePhone();
        takeRollNo();
        while (!getFingerprintEnroll());
        writeRegisterPacket();
        readRegisterResponsePacket();
        if (auth == 1)
        {
            bool res = storeTemplate();
            if (res == true)
                Serial.println("Registered");
            else
                Serial.println("Problem in storing");
        }
        else
        {
            Serial.println("Who tf r u");
        }
    }

    else if (inp == 'B')
    {
        Serial.println("Mark Attendance");
        while (!getFingerprintID());
        writeAttendancePacket();
        readAttendanceResponsePacket();
    }
    else
        Serial.println("Invalid Input");
}