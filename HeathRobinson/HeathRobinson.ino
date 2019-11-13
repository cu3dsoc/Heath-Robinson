#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h>

#define COIN_COUNTER_PULSE 2
#define EEPROM_RST_PIN 3
#define SERVO_PIN 6

#define SERVO_CLOSED 90
#define SERVO_OPEN 180

#define MAX_PULSES 6
// Milliseconds to wait for pulses before stopping counting
#define THRESH_PULSE_WAIT 200

// LCD Debugging
#define DEBUG true

// Minimum spend (in pence) to make servo open
#define MIN_SPEND 50

// EEPROM addresses
#define TOTAL_POUND_ADDR 1
#define TOTAL_PENCE_ADDR 2

// Initialise vars
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
Servo servo;

float i;
int totalPence = EEPROM.read(TOTAL_POUND_ADDR) * 100;
int tempCount = 0;
volatile float pulses = 0;
volatile bool resetFlag = false;
unsigned long timeOfLastPulse = 0;

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

// Conversion from number of pulses to pence
int penceLookup[MAX_PULSES] = {
  5,
  10,
  20,
  50,
  100,
  200
};


void setup()
{
  tempCount = EEPROM.read(TOTAL_PENCE_ADDR);
  totalPence += tempCount;

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println("Coin Acceptor Ready!");

  // set up the LCD's number of columns and rows:
  //  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.createChar(0, pound);

  // Attach servo to servo pin and set to 0 degrees
  servo.attach(SERVO_PIN);
  servo.write(SERVO_CLOSED);

  pinMode(13, OUTPUT);

  pinMode(EEPROM_RST_PIN, INPUT_PULLUP);
  pinMode(COIN_COUNTER_PULSE, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EEPROM_RST_PIN), resetISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(COIN_COUNTER_PULSE), pulseRegistered, FALLING);

  displayTotal();
  resetFlag = false;
}

void loop()
{
  displayTotal();
  // Wait until we get a coin or we need to reset
  while(digitalRead(COIN_COUNTER_PULSE) == HIGH
        && resetFlag == false) {} 

  // If we broke the loop because of a reset, do the reset.
  // Else, it was because of a coin, so process that.
  if(resetFlag == true){
    Serial.println(resetFlag);
    reset();
  }
  else {
    int pulses = countPulses();
    Serial.print("Pulses: ");
    Serial.println(pulses);
  
    int pence;
    if(pulses <= MAX_PULSES){
      pence = penceLookup[pulses - 1];
    }
  
    totalPence += pence;
    tempCount += pence;
  
    EEPROM.write(TOTAL_POUND_ADDR, totalPence / 100);
    EEPROM.write(TOTAL_PENCE_ADDR, totalPence % 100);
  
    Serial.print("Amount inserted: ");
    Serial.println(poundsFromPence(totalPence));
  
    // clear the LCD
    lcd.clear();
    lcd.write(byte(0));
    lcd.setCursor(1, 0);
    lcd.print(poundsFromPence(tempCount) + " Inserted");
  
    if (tempCount >= MIN_SPEND)
    {
      displayTotal();
      openServo();
      tempCount = 0;
    }
  }
}

// Return string containing pounds from pence
String poundsFromPence(int pence)
{
  // If it's an exact pound
//  if(pence % 100 == 0){
//    return (String)(pence / 100) + ".00";
//  }
//  // If it's an exact 10 (eg £5.60)
//  else if(pence % 100 == 10){
//    return (String)(pence / 100) + ".0";
//  }
//  return (String)(pence / 100) + "." + (String)(pence % 100);
    return (String)(pence / 100) + "." + padPence(pence % 100);
}

// Return '05' for 5 pence, '10' for 10 pence etc
String padPence(int pence)
{
 if(pence < 10){
   return "0" + (String)pence;
 }
 return (String)pence;
}

// Count the number of pulses sent by the coin counter.
// This function should be triggered just after the pin has gone LOW for the
// first pulse.
int countPulses()
{
  timeOfLastPulse = millis();
  while(millis() - timeOfLastPulse < THRESH_PULSE_WAIT){}
  int result = pulses;
  // Reset for next time
  pulses = 0;
  return result;
}

void pulseRegistered() 
{
  timeOfLastPulse = millis();
  pulses += 1;
}

void resetISR()
{
 resetFlag = true; 
}

void reset()
{
  EEPROM.write(TOTAL_POUND_ADDR, 0);
  EEPROM.write(TOTAL_PENCE_ADDR, 0);
  totalPence = 0;

  Serial.println("Resetting");
  lcd.clear();
  lcd.print("Reset");
  delay(1000);
  lcd.clear();
  displayTotal();
  resetFlag = false;
}

void openServo() 
{
  servo.write(SERVO_OPEN);
  Serial.println("Servo open");

  delay(3000);

  //Clear LCD and close servo
  lcd.clear();
  servo.write(SERVO_CLOSED);
  Serial.println("Servo closed");
}

void displayTotal()
{
  Serial.print("Displaying total");
  Serial.print("Total: £");
  Serial.println(poundsFromPence(totalPence));
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  lcd.print(poundsFromPence(totalPence) + " Donated");

  lcd.setCursor(0, 0);
  lcd.print("Insert donation");
}
