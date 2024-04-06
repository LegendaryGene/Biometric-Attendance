#include <LiquidCrystal.h>

#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

const byte ROWS = 4; 
const byte COLS = 4;

const int LCD_RS=A5;
const int LCD_EN=A4;
const int LCD_D4=A3;
const int LCD_D5=A2;
const int LCD_D6=A1;
const int LCD_D7=A0;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {13, 12, 11, 10}; 
byte colPins[COLS] = {9, 8, 7, 6}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

SoftwareSerial blueSerial(2, 3);
SoftwareSerial A9Serial(RX_PIN, TX_PIN);

char password[11];
char phone[11];
char rollno[7];
uint8_t id=0;
uint8_t id_detected=0;
char recvd_phone[10];
char recvd_rollno[6];
char recvd_date[8];

void lcdPrint(String top, String bottom){
  lcd.setCursor(0,0);
  lcd.print(top);
  lcd.setCursor(0,1);
  lcd.print(bottom);
}

void writeRegisterPacket(){
    char packet[27];
    packet[0] = 0x42;
    int idx = 1;
    for(int i = 0; i < 6; i++) packet[idx++] = rollno[i];
    for(int i = 0; i < 10; i++) packet[idx++] = phone[i];
    for(int i = 0; i < 10; i++) packet[idx++] = password[i];
    blueSerial.write(packet);
}

void readRegisterResponsePacket(){
    char res[10];
    uint8_t nRead = 0;
    blueSerial.listen():
    while(nRead != 10){
        if(blueSerial.available() > 0){
            res[nRead++] = blueSerial.read();
        }
    }
    for(int i = 0; i < 10; i++) Serial.print(res[i]);
}

void clearDisplay(){
  String clearstring = "                ";
  lcdPrint(clearstring, clearstring);
}

void writeAttendancePacket(){
    char packet[5];
    packet[0] = 0x42;
    packet[1] = (uint8_t)(id_detected >> 24);
    packet[2] = (uint8_t)(id_detected >> 16);
    packet[3] = (uint8_t)(id_detected >> 8);
    packet[4] = (uint8_t)(id_detected);
    blueSerial.write(packet);
}

char taskInput(){
    clearDisplay();
    lcdPrint("A - Register", "B - Mark Attnd");
    Serial.println("Register(A) or Mark Attendance(B)?");
    char inp = customKeypad.waitForKey();
    return inp;
}

void clean(char *buf, int len){
  for(int i=0;i<len;i++){
    buf[i]='\0';
  }
}

byte takePassword(){
    clearDisplay();
    clean(password, 10);  
    Serial.println("Enter Admin Password");
    String top = "Enter Adm Pwd";
    lcdPrint(top, "");
    byte idx = 0;
    char c = 0;
    while(idx < 10 && c != 'D'){
        c = customKeypad.waitForKey();
        if(c == 'D'){
            Serial.println();
            break;
        }
        Serial.print(c);
        password[idx] = c;
        lcdPrint(top, password);
        idx++;
    }
    if(idx!=10){
      for(int i = idx; i < 11 i++) password[i] = 'x';
    }
    return idx;
}

void takePhone(){
    Serial.println("Enter Phone Number");
    clearDisplay();
    clean(phone, 10);
    String top = "Enter Phone No.";
    lcdPrint(top, "");
    byte idx = 0;
    char c = 0;
    while(idx < 11 && c != 'D'){
        c = customKeypad.waitForKey();
        if(c == 'D'){
            Serial.println();
            break;
        }
        Serial.print(c);
        phone[idx] = c;
        lcdPrint(top, phone);
        idx++;
    }
    for(int i = idx; i < 11; i++) phone[i] = 'x';
}

void takeRollNo(){
    Serial.println("Enter Roll Number");
    clearDisplay();
    clean(rollno, 6);
    String top = "Enter Roll No.";
    lcdPrint(top, "");
    byte idx = 0;
    char c = 0;
    while(idx < 7 && c != 'D'){
        c = customKeypad.waitForKey();
        if(c == 'D'){
            Serial.println();
            break;
        }
        Serial.print(c);
        rollno[idx] = c;
        lcdPrint(top, rollno);
        idx++;
    }
    for(int i = idx; i < 7; i++) rollno[i] = 'x';
}

void SendMessage(int mode){
  A9Serial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  if(mode==0){
    A9Serial.println(String("AT+CMGS=\"+91")+phone+String("\"\r")); // Replace x with mobile number
    delay(1000);
    A9Serial.println(String("Roll Number ")+rollno+String(" has been registered on ")+recvd_date);// The SMS text you want to send
    delay(100);
    A9Serial.println((char)26);// ASCII code of CTRL+Z
    delay(1000);
  }
  else{
    A9Serial.println(String("AT+CMGS=\"+91")+recvd_phone+String("\"\r")); // Replace x with mobile number
    delay(1000);
    A9Serial.println(String("Roll Number ")+recvd_rollno+String(" has been marked present for ")+recvd_date);// The SMS text you want to send
    delay(100);
    A9Serial.println((char)26);// ASCII code of CTRL+Z
    delay(1000);
  }
}

//uint8_t getFingerprintEnroll() {
//
//  int p = -1;
//  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
//  while (p != FINGERPRINT_OK) {
//    p = finger.getImage();
//    switch (p) {
//    case FINGERPRINT_OK:
//      Serial.println("Image taken");
//      break;
//    case FINGERPRINT_NOFINGER:
//      Serial.print(".");
//      break;
//    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
//      break;
//    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
//      break;
//    default:
//      Serial.println("Unknown error");
//      break;
//    }
//  }
//
//  // OK success!
//
//  p = finger.image2Tz(1);
//  switch (p) {
//    case FINGERPRINT_OK:
//      Serial.println("Image converted");
//      break;
//    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
//      return p;
//    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
//      return p;
//    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
//      return p;
//    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
//      return p;
//    default:
//      Serial.println("Unknown error");
//      return p;
//  }
//
//  Serial.println("Remove finger");
//  delay(2000);
//  p = 0;
//  while (p != FINGERPRINT_NOFINGER) {
//    p = finger.getImage();
//  }
//  Serial.print("ID "); Serial.println(id);
//  p = -1;
//  Serial.println("Place same finger again");
//  while (p != FINGERPRINT_OK) {
//    p = finger.getImage();
//    switch (p) {
//    case FINGERPRINT_OK:
//      Serial.println("Image taken");
//      break;
//    case FINGERPRINT_NOFINGER:
//      Serial.print(".");
//      break;
//    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
//      break;
//    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
//      break;
//    default:
//      Serial.println("Unknown error");
//      break;
//    }
//  }
//
//  // OK success!
//
//  p = finger.image2Tz(2);
//  switch (p) {
//    case FINGERPRINT_OK:
//      Serial.println("Image converted");
//      break;
//    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
//      return p;
//    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
//      return p;
//    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
//      return p;
//    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
//      return p;
//    default:
//      Serial.println("Unknown error");
//      return p;
//  }
//
//  // OK converted!
//  Serial.print("Creating model for #");  Serial.println(id);
//
//  p = finger.createModel();
//  if (p == FINGERPRINT_OK) {
//    Serial.println("Prints matched!");
//  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
//    return p;
//  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
//    Serial.println("Fingerprints did not match");
//    return p;
//  } else {
//    Serial.println("Unknown error");
//    return p;
//  }
//
//  return true;
//}

void setup(){
    Serial.begin(9600);
    blueSerial.begin(9600);
    A9Serial.begin(115200);
    lcd.begin(16,2);
//    String out= "top";
//    String down = "down";
//    char cc[3];
//    cc[0]='A';cc[1]='B';cc[2]='\0';
//    lcdPrint(out, cc);
      lcdPrint("Welcome to", "Fingering");
}

void loop(){

    
     char inp = taskInput();
     switch(inp){
         case 'A':
             Serial.println("Registration Request Initiated");
             byte len = takePassword();
             takePhone();
             takeRollNo(); 
 //            while(!getFingerprintEnroll());
             writeRegisterPacket();
             break;
         case 'B':
             Serial.println("Mark Attendance");
 //            while(!getFingerprintEnroll());
             writeAttendancePacket();
             break;
         default:
             Serial.println("Invalid Input");
     }
}

