#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

const byte ROWS = 4;
const byte COLS = 4;

LiquidCrystal_I2C lcd(0x27, 16, 2);

char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {9, 8, 7, 6};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
SoftwareSerial blueSerial(2, 3);
SoftwareSerial A9Serial(A0, A1);
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
    Serial.println("Sent");
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
    String clearstring = F("                ");
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
        recvd_date[i - 1] = res[i];
    for (int i = 9; i < 15; i++)
        recvd_rollno[i - 9] = res[i];
    for (int i = 15; i < 25; i++)
        recvd_phone[i - 15] = res[i];
}

char taskInput()
{
    clearDisplay();
    lcdPrint(F("A - Register"), F("B - Mark Attndce"));
    Serial.println(F("Register(A) or Mark Attendance(B)?"));
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
    Serial.println(F("Enter Admin Password"));
    String top = F("Enter Adm Pwd");
    lcdPrint(top, F(""));
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
    Serial.println(F("Enter Phone Number"));
    clearDisplay();
    clean(phone, 10);
    String top = F("Enter Phone No.");
    lcdPrint(top, F(""));
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
    Serial.println(F("Enter Roll Number"));
    clearDisplay();
    clean(rollno, 6);
    String top = F("Enter Roll No.");
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
    A9Serial.listen();
    A9Serial.println(F("AT+CMGF=1")); // Sets the GSM Module in Text Mode
    delay(1000);
    if (A9Serial.available() > 0)
    {
        Serial.println(A9Serial.readString());
    }
    else
        Serial.println(F("Not available"));
    delay(1000); // Delay of 1000 milli seconds or 1 second
    if (mode == 0)
    {
        String phonenum = F("");
        String rollnumber = F("");
        String datetoday = F("");
        for (int i = 0; i < 10; i++)
            phonenum += phone[i];
        for (int i = 0; i < 6; i++)
            rollnumber += rollno[i];
        for (int i = 0; i < 8; i++)
            datetoday += recvd_date[i];
        //      Serial.println(phonenum);
        //      Serial.println(rollnumber);
        A9Serial.println(String(F("AT+CMGS=\"+91")) + phonenum + String(F("\"\r")));
        delay(1000);
        A9Serial.println(String(F("Roll Number ")) + rollnumber + String(F(" has been registered on ")) + datetoday);
        delay(1000);
        A9Serial.println((char)26); // ASCII code of CTRL+Z
        delay(1000);
    }
    else
    {
        String phonenum = F("");
        String rollnumber = F("");
        String datetoday = F("");
        for (int i = 0; i < 10; i++)
            phonenum += recvd_phone[i];
        for (int i = 0; i < 6; i++)
            rollnumber += recvd_rollno[i];
        for (int i = 0; i < 8; i++)
            datetoday += recvd_date[i];
        //      Serial.println(phonenum);
        //      Serial.println(rollnumber);
        //      Serial.println(datetoday);
        A9Serial.println(String(F("AT+CMGS=\"+91")) + phonenum + String(F("\"\r")));
        delay(1000);
        A9Serial.println(String(F("Roll Number ")) + rollnumber + String(F(" has been marked present for ")) + datetoday);
        delay(1000);
        A9Serial.println((char)26); // ASCII code of CTRL+Z
        delay(1000);
    }
    if (A9Serial.available() > 0)
    {
        Serial.println(A9Serial.readString());
    }
}

uint8_t getFingerprintEnroll()
{

    FingerSerial.listen();
    int p = -1;
    Serial.print(F("Waiting for valid finger to enroll as #"));
    Serial.println(id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println(F("Image taken"));
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println(F("Communication error"));
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println(F("Imaging error"));
            break;
        default:
            Serial.println(F("Unknown error"));
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println(F("Image converted"));
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println(F("Image too messy"));
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println(F("Could not find fingerprint features"));
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println(F("Could not find fingerprint features"));
        return p;
    default:
        Serial.println(F("Unknown error"));
        return p;
    }

    Serial.println(F("Remove finger"));
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(id);
    p = -1;
    Serial.println(F("Place same finger again"));
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            Serial.println(F("Image taken"));
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println(F("Communication error"));
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println(F("Imaging error"));
            break;
        default:
            Serial.println(F("Unknown error"));
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println(F("Image converted"));
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println(F("Image too messy"));
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println(F("Could not find fingerprint features"));
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println(F("Could not find fingerprint features"));
        return p;
    default:
        Serial.println(F("Unknown error"));
        return p;
    }

    // OK converted!
    Serial.print(F("Creating model for #"));
    Serial.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        Serial.println(F("Prints matched!"));
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println(F("Communication error"));
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        Serial.println(F("Fingerprints did not match"));
        return false;
    }
    else
    {
        Serial.println(F("Unknown error"));
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
        Serial.println(F("Stored!"));
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println(F("Communication error"));
        return false;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        Serial.println(F("Could not store in that location"));
        return false;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        Serial.println(F("Error writing to flash"));
        return false;
    }
    else
    {
        Serial.println(F("Unknown error"));
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
    A9Serial.print(F("AT+CMGF=1\r"));
    delay(100);
    lcd.init();
    lcd.backlight();
    finger.begin(57600);
    if (finger.verifyPassword())
    {
        Serial.println(F("Found fingerprint sensor!"));
    }
    else
    {
        Serial.println(F("Did not find fingerprint sensor :("));
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
        Serial.println(F("Image taken"));
        break;
    case FINGERPRINT_NOFINGER:
        Serial.println(F("No finger detected"));
        return false;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        return p;
    case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        return p;
    default:
        Serial.println(F("Unknown error"));
        return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.println(F("Image converted"));
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println(F("Image too messy"));
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println(F("Could not find fingerprint features"));
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println(F("Could not find fingerprint features"));
        return p;
    default:
        Serial.println(F("Unknown error"));
        return p;
    }

    // OK converted!
    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK)
    {
        Serial.println(F("Found a print match!"));
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        Serial.println(F("Communication error"));
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        Serial.println(F("Did not find a match"));
        return false;
    }
    else
    {
        Serial.println(F("Unknown error"));
        return p;
    }

    // found a match!
    Serial.print(F("Found ID #"));
    Serial.print(finger.fingerID);
    Serial.print(F(" with confidence of "));
    Serial.println(finger.confidence);

    id_detected = finger.fingerID;
    return true;
}

void loop()
{
    lcdPrint("Welcome to", "Advanced Attndce");
    delay(2000);
    char inp = taskInput();
    Serial.println(inp);
    if (inp == 'A')
    {
        clearDisplay();
        lcdPrint(F("Registering"), F(""));
        delay(2000);
        Serial.println(F("Registration Request Initiated"));
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
            {
                clearDisplay();
                lcdPrint(F("Registered"), rollno);
                delay(2000);
                Serial.println(F("Registered"));
                SendMessage(0);
            }
            else
            {
                clearDisplay();
                lcdPrint(F("Error in"), F("Storing"));
                delay(2000);
                Serial.println(F("Problem in storing"));
            }
        }
        else
        {
            clearDisplay();
            lcdPrint(F("Wrong"), F("Password"));
            delay(2000);
        }
    }

    else if (inp == 'B')
    {
        clearDisplay();
        lcdPrint(F("Place"), F("Finger"));
        Serial.println(F("Mark Attendance"));
        while (!getFingerprintID());
        writeAttendancePacket();
        readAttendanceResponsePacket();
        SendMessage(1);
    }
    else
        Serial.println(F("Invalid Input"));
}