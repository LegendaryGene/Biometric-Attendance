#include <Keypad.h>

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

char password[10];

char taskInput(){
    Serial.println("Register(A) or Mark Attendance(B)?");
    char inp = customKeypad.waitForKey();
    return inp;
}

int takePassword(){
    Serial.println("Enter Admin Password");
    int idx = 0;
    char c = 0;
    while(idx < 10 && c != 'D'){
        c = customKeypad.waitForKey();
        if(c == 'D') break;
        password[idx] = c;
        idx++;
    }
}

void setup(){
    Serial.begin(9600);
}

void loop(){
    char inp = taskInput();
    switch(inp){
        case 'A':
            Serial.println("Registration Request Initiated");
            takePassword();
            
            break;
        case 'B':
            Serial.println("Req to finger");
            break;
        default:
            Serial.println("Invalid Input");
    }
}
