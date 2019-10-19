 #include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h>

#define INHIBIT_PIN 13
#define EEPROM_RST_PIN 7
#define SERVO_PIN 6
#define COIN_COUNTER_RX 0
#define COIN_COUNTER_TX 1

#define SERVO_CLOSED 90
#define SERVO_OPEN 180

// LCD Debugging
#define DEBUG true

// Minimum spend
#define MIN_SPEND 0.5

// EEPROM addresses
#define COUNTER_ADDR 0
#define TOTAL_ADDR 1


// Initialise vars
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
SoftwareSerial coinSerial(COIN_COUNTER_RX, COIN_COUNTER_TX);
Servo servo;

float i;
int counter = EEPROM.read(COUNTER_ADDR);
float total = EEPROM.read(TOTAL_ADDR);

float tempCount = 0.0;

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
  
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Coin Acceptor Ready!");

  // set up the LCD's number of columns and rows:
  //  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.createChar(0, pound);
    
  // set the data rate for the SoftwareSerial port
  coinSerial.begin(4800);
  
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
  if (coinSerial.available()) {

    // read input, which is a 1 byte integer
    i = coinSerial.read();
    i = i/100;
    counter += 1;
    total += i;
    tempCount += i;

    EEPROM.write(COUNTER_ADDR, counter);
    EEPROM.write(TOTAL_ADDR, total);

    Serial.print("Amount inserted: ");
    Serial.println(i);
    Serial.print("Counter:");
    Serial.println(counter);

    // clear the LCD
    lcd.clear();
    lcd.write(byte(0));
    lcd.setCursor(1,0);
    lcd.print(String(tempCount) + " Inserted");

    if(tempCount > MIN_SPEND){
            
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
    EEPROM.write(COUNTER_ADDR,0);
    EEPROM.write(TOTAL_ADDR,0);
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
