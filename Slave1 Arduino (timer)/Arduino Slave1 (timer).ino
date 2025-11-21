// Slave 2 for Skeeball Machine Timer
#include <Wire.h> 
// 7-segment display pin assignments
int pinA = 2;
int pinB = 3;
int pinC = 4;
int pinD = 5;
int pinE = 6;
int pinF = 7;
int pinG = 8;
int D1 = 9;
int D2 = 10;
int D3 = 11;
int D4 = 12;

// Timer variables
unsigned long startTime;
unsigned long elapsedTime;
bool timerRunning = false;
int countdownSeconds = 100;

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
  
  // Turn off all digits initially
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  
  Serial.begin(9600);
  Serial.println("Timer Ready - Send seconds via Serial");
  // Start the I2C Bus as Slave on address 10
  Wire.begin(10);
  // Attach a function to trigger when something is received. 
  Wire.onReceive(receiveEvent);
  
  // ADD THIS LINE to automatically start the timer
  startTimer(countdownSeconds);
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

void displayNumber(int number) {
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

void startTimer(int seconds) {
  countdownSeconds = seconds;
  startTime = millis();
  timerRunning = true;
  Serial.print("Timer started: ");
  Serial.print(seconds);
  Serial.println(" seconds");
}

void updateTimer() {
  if(!timerRunning) return;
  
  elapsedTime = (millis() - startTime) / 1000;
  int remaining = countdownSeconds - elapsedTime;
  
  if(remaining >= 0) {
    displayNumber(remaining);
    
    if(remaining == 0) {
      // Timer finished - flash display
      for(int i = 0; i < 10; i++) {
        displayNumber(0);
        delay(500);
        clearDisplay();
        delay(500);
      }
      timerRunning = false;
      Serial.println("TIMER DONE!");
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
  // Check for serial input to start timer
  if(Serial.available() > 0) {
    int seconds = Serial.parseInt();
    if(seconds > 0 && seconds <= 9999) {
      startTimer(seconds);
    }
  }
  
  updateTimer();
  delay(100);
}

void receiveEvent() {
  Wire.write(2); // Respond with 1 when requested
}