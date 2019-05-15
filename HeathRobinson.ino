#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Servo.h>

#define INHIBIT_PIN 13
#define EEPROM_RST_PIN 7
#define SERVO_PIN 6
#define SOFT_SER_RX 0
#define SOFT_SER_TX 1

#define SERVO_CLOSED 90
#define SERVO_OPEN 180


SoftwareSerial mySerial(SOFT_SER_RX, SOFT_SER_TX);
Servo servo;

int C_addr = 0; //EEprom counter address
int T_addr = 1; //EEprom total address


//Initialise values
float i;
int counter = EEPROM.read(C_addr);
float total = EEPROM.read(T_addr);

float minSpend = 0.50;
float tempCount = 0.0;  

//LCD Debugging
bool DEBUG = true;

byte pound[8] = {
  0b00110,
  0b01000,
  0b01000,
  0b11100,
  0b01000,
  0b01001,
  0b10110,
  0b00000
};

const int rs = 11, en = 12, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

  
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Coin Acceptor Ready!");

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, pound);
    
  // set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  
  // Turn inhibit off
  digitalWrite(INHIBIT_PIN, LOW);

  // Attach servo to servo pin and set to 0 degrees
  servo.attach(SERVO_PIN);
  servo.write(SERVO_CLOSED);

  pinMode(13, OUTPUT);
  
  pinMode(EEPROM_RST_PIN, INPUT);
  
  if(DEBUG == true) {
    Display_total();    
  }

  
}

void loop() {
  // any input coming from coin acceptor?
  if (mySerial.available()) {

    // read input, which is a 1 byte integer
    i = mySerial.read();
    i = i/100;
    counter += 1;
    total += i;
    tempCount += i;

    EEPROM.write(C_addr, counter);
    EEPROM.write(T_addr, total);

    Serial.print("Amount inserted: ");
    Serial.println(i);
    Serial.print("Counter:");
    Serial.println(counter);

    // clear the LCD
    lcd.clear();
    lcd.write(byte(0));
    lcd.setCursor(1,0);
    lcd.print(String(tempCount) + " Inserted");

    if(tempCount > minSpend){
            
      // Inhibit coin sorter
      digitalWrite(INHIBIT_PIN, HIGH);
          
      //Open servo
      delay(1000);
      servo.write(SERVO_OPEN);
      digitalWrite(13,HIGH);
      Serial.println("Servo open");
  
  
      
      delay(3000);
  
      //Clear LCD and close servo
      lcd.clear();
      servo.write(SERVO_CLOSED);
      digitalWrite(13,LOW);
      Serial.println("Servo closed");
  
      // Re open coin sorter
      digitalWrite(INHIBIT_PIN, LOW);
  
      if(DEBUG == true) {
        Display_total();      
      }
    }
  }
  
  if(digitalRead(EEPROM_RST_PIN) == true) {
    EEPROM.write(C_addr,0);
    EEPROM.write(T_addr,0);
    counter = 0;
    total = 0;

    Serial.println("Reset");
    lcd.clear();
    lcd.print("Reset");
    delay(1000);
    lcd.clear();
    Display_total();
  }
}

void Display_total() {
    
    lcd.setCursor(0,1);
    lcd.write(byte(0));
    lcd.setCursor(1,1);
    lcd.print(String(total) + " Donated");
    
    lcd.setCursor(0,0);
    lcd.print("Insert donation");
}
