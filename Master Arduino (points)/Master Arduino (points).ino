#include <Wire.h>
#define SLAVE_SENSORS 0x08
#define SLAVE_TIMER 0x09

// 7-segment display pin assignments
const int pinA = 2;
const int pinB = 3;
const int pinC = 4;
const int pinD = 5;
const int pinE = 6;
const int pinF = 7;
const int pinG = 8;
const int D1 = 9;
const int D2 = 10;
const int D3 = 11;
const int D4 = 12;

// Digit patterns for 0-9 (common cathode)
byte digitPatterns[10] = {
  B11111100, // 0 - ABCDEF
  B01100000, // 1 - BC
  B11011010, // 2 - ABDEG
  B11110010, // 3 - ABCDG
  B01100110, // 4 - BCFG
  B10110110, // 5 - ACDFG
  B10111110, // 6 - ACDEFG  
  B11100000, // 7 - ABC
  B11111110, // 8 - ABCDEFG
  B11110110  // 9 - ABCDFG
};

// Variable to store the received number from slave
int receivedNumberT = 0;
int receivedNumberP = 0;
bool gameEnded = false;

void setup() {
  // Initialize all pins as outputs
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  pinMode(pinE, OUTPUT);
  pinMode(pinF, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);

  // pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Turn off all digits initially
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  
  // I2C Master setup
  Wire.begin();           // Join I2C bus as master
  Serial.begin(9600);     // Start serial for output
  Serial.println("Master started");
}

void displayDigit(int digit, int position) {
  // Turn off all digits
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  
  // Set segment patterns
  byte pattern = digitPatterns[digit];
  digitalWrite(pinA, bitRead(pattern, 7));
  digitalWrite(pinB, bitRead(pattern, 6));
  digitalWrite(pinC, bitRead(pattern, 5));
  digitalWrite(pinD, bitRead(pattern, 4));
  digitalWrite(pinE, bitRead(pattern, 3));
  digitalWrite(pinF, bitRead(pattern, 2));
  digitalWrite(pinG, bitRead(pattern, 1));
  
  // Turn on the specific digit
  switch(position) {
    case 1: digitalWrite(D1, LOW); break;
    case 2: digitalWrite(D2, LOW); break;
    case 3: digitalWrite(D3, LOW); break;
    case 4: digitalWrite(D4, LOW); break;
  }
}

void displayReceivedNumber(int number) {
  int digits[4];
  
  // Extract individual digits
  digits[0] = number / 1000;        // Thousands
  digits[1] = (number / 100) % 10;  // Hundreds
  digits[2] = (number / 10) % 10;   // Tens
  digits[3] = number % 10;          // Units
  
  // Display each digit briefly (multiplexing)
  for(int i = 0; i < 4; i++) {
    if(digits[i] != 0 || i > 0) { // Don't display leading zeros
      displayDigit(digits[i], i+1);
      delay(3); // Adjust for brightness/flicker
    }
  }
}

void clearDisplay() {
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
}


void loop() {
  if (gameEnded == false) {
    // Continuously display the received score from Slave 2
    displayReceivedNumber(receivedNumberP);
  } else {
    // Timer finished - flash display
      for(int i = 0; i < 20; i++) {
        displayReceivedNumber(receivedNumberP);
    }
  }

  static unsigned long lastI2CRead_Sensor = 0;
  // Send timer value to Slave 2 every second
  if (millis() - lastI2CRead_Sensor >= 350) {
    lastI2CRead_Sensor = millis();
    // Read from I2C slave sensor every 2 seconds
    Wire.requestFrom(SLAVE_SENSORS, 4);  // Request 4 bytes
    
    if (Wire.available() >= 4) {
      int firstValue = Wire.read();
      int secondValue = Wire.read();
      int thirdValue = Wire.read();
      int fourthValue = Wire.read();
      
      /*
      Serial.print("Values: ");
      Serial.print(firstValue);
      Serial.print(", ");
      Serial.print(secondValue);
      Serial.print(", ");
      Serial.print(thirdValue);
      Serial.print(", ");
      Serial.println(fourthValue);\
      */

      receivedNumberP = (firstValue * 1000) + (secondValue * 100) + (thirdValue * 10) + (fourthValue);
    }
  } 


  // Read from I2C slave timer every 2 seconds
  static unsigned long lastI2CRead_Timer = 0;
  if (millis() - lastI2CRead_Timer >= 500) {
    lastI2CRead_Timer = millis();

    Wire.requestFrom(SLAVE_TIMER, 4);  // Request 4 bytes from slave timer

    if (Wire.available() == 4) {
      // Read all four bytes (each should be 0)
      byte digit1 = Wire.read();
      byte digit2 = Wire.read();
      byte digit3 = Wire.read();
      byte digit4 = Wire.read();

      receivedNumberT = (digit1 * 1000) + (digit2 * 100) + (digit3 * 10) + (digit4);
    }
  }

  static unsigned long lastI2CSend_S = 0;
  if (millis() - lastI2CSend_S >= 1000) {
    lastI2CSend_S = millis();

    // Send timer value to Slave 2 (single byte)
    Wire.beginTransmission(SLAVE_SENSORS);
    Wire.write(receivedNumberT);  // Just send the value directly
    Wire.endTransmission();

    Serial.print("Sent timer to Slave 2: ");
    Serial.println(receivedNumberT);

    if (receivedNumberT == 0) {
      gameEnded = true;
      }
  }
}