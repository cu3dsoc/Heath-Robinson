#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Servo.h>

SoftwareSerial mySerial(0,1); // RX, TX
int inhibit_pin = 13;

Servo servo;
int servoPin = 6;
int servo_closed = 90;
int servo_open = 180;

int EE_rst = 7;      //EEPROM reset pin

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
  digitalWrite(inhibit_pin, LOW);

  // Attach servo to servo pin and set to 0 degrees
  servo.attach(servoPin);
  servo.write(servo_closed);

  pinMode(13, OUTPUT);
  
  pinMode(EE_rst, INPUT);
  
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
      digitalWrite(inhibit_pin, HIGH);
          
      //Open servo
      delay(1000);
      servo.write(servo_open);
      digitalWrite(13,HIGH);
      Serial.println("Servo open");
  
  
      
      delay(3000);
  
      //Clear LCD and close servo
      lcd.clear();
      servo.write(servo_closed);
      digitalWrite(13,LOW);
      Serial.println("Servo closed");
  
      // Re open coin sorter
      digitalWrite(inhibit_pin, LOW);
  
      if(DEBUG == true) {
        Display_total();      
      }
    }
  }
  
  if(digitalRead(EE_rst) == true) {
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
