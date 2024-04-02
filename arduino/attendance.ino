#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

const byte ROWS = 4; 
const byte COLS = 4; 

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

char password[10];
char phone[10];
char rollno[6];

void writeRegisterPacket(){
    char packet[27];
    packet[0] = 0x42;
    int idx = 1;
    for(int i = 0; i < 6; i++) packet[idx++] = rollno[i];
    for(int i = 0; i < 10; i++) packet[idx++] = phone[i];
    for(int i = 0; i < 10; i++) packet[idx++] = password[i];
    blueSerial.write(packet);
}

char taskInput(){
    Serial.println("Register(A) or Mark Attendance(B)?");
    char inp = customKeypad.waitForKey();
    return inp;
}

byte takePassword(){
    Serial.println("Enter Admin Password");
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
        idx++;
    }
    for(int i = idx; i < 10; i++) password[i] = 'x';
    return idx;
}

void takePhone(){
    Serial.println("Enter Phone Number");
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
        idx++;
    }
    for(int i = idx; i < 10; i++) phone[i] = 'x';
}

void takeRollNo(){
    Serial.println("Enter Roll Number");
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
        idx++;
    }
    for(int i = idx; i < 6; i++) rollno[i] = 'x';
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
            Serial.println("Req to finger");
            break;
        default:
            Serial.println("Invalid Input");
    }
}