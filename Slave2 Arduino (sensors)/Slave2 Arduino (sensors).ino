#include <Wire.h>
#define SLAVE_SENSORS 0x08

// Dual IR Sensors + Ultrasonic Sensor Counter (FIXED)
const int irSensorPin1 = 9;
const int irSensorPin2 = 10;
const int irSensorPin3 = 13;
const int trigPin1 = 12;
const int echoPin1 = 11;
const int trigPin2 = 7;
const int echoPin2 = 8;

/*
// L298N Motor Controller connections
int enB = 5;
int in1 = 4;
int in2 = 3;
*/

/*
const int RESET_COMMAND = 0xFF;
bool resetRequested = false;
*/

const int SHORT_DISTANCE = 12;
const int SHORT_DISTANCE2 = 12;

int counter = 0;
int digitsToMaster0 = 0;
int digitsToMaster1 = 0;
int digitsToMaster2 = 0;
int digitsToMaster3 = 0;
bool programRunning = true;
int timerRemaining = -1;  // Store timer value received from master
bool gameEnded = false;

// Separate timing for each sensor type
unsigned long lastIRDetectionTime = 0;
unsigned long lastUltrasonicTime = 0;
const unsigned long irDebounceDelay = 300;      // IR sensors
const unsigned long ultrasonicDelay = 500;      // Ultrasonic sensor

void setup() {
  pinMode(irSensorPin1, INPUT);
  pinMode(irSensorPin2, INPUT);
  pinMode(irSensorPin3, INPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  Serial.begin(9600);

  Wire.begin(SLAVE_SENSORS);        // Join I2C bus as slave with address 0x08
  Wire.onRequest(requestEvent);     // Register callback for when master requests data
  Wire.onReceive(receiveEvent);     // Register callback fopr when master sends data
}

// I2C receive event - receives timer data from master
void receiveEvent(int howMany) {
  if (Wire.available()) {
    timerRemaining = Wire.read();

    // Close door when timer reaches 0 and game hasn't ended
    if (timerRemaining == 0 && !gameEnded) {
      gameEnded = true;
    }

  }
}

long getUltrasonicDistance1() {
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  
  long duration = pulseIn(echoPin1, HIGH, 4000);
  
  if (duration == 0) {
    return -1; // No echo received
  }
  
  long distance = duration * 0.0343 / 2;
  return distance;
}

long getUltrasonicDistance2() {
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  
  long duration = pulseIn(echoPin2, HIGH, 4000);
  
  if (duration == 0) {
    return -1; // No echo received
  }
  
  long distance = duration * 0.0343 / 2;
  return distance;
}

void processUltrasonicSensor1() {
  if (gameEnded) return; // Stop processing sensors when game ended
  
  unsigned long currentTime = millis();
  
  // Check ultrasonic less frequently to give IR sensors more time
  if (currentTime - lastUltrasonicTime > ultrasonicDelay) {
    long distance = getUltrasonicDistance1();
    
    if (distance >= 2 && distance <= 400) {
      if (distance <= SHORT_DISTANCE) {
        counter += 50;
        Serial.println(">>> ULTRASONIC1: SHORT RANGE! <<<");
        Serial.println("Distance: " + String(distance) + "cm - Added 50");
      }
      Serial.println("Total Counter: " + String(counter));
      Serial.println("------------------------");
      lastUltrasonicTime = currentTime;
    }
  }
}

void processUltrasonicSensor2() {
  if (gameEnded) return; // Stop processing sensors when game ended
  
  unsigned long currentTime = millis();
  
  // Check ultrasonic less frequently to give IR sensors more time
  if (currentTime - lastUltrasonicTime > ultrasonicDelay) {
    long distance = getUltrasonicDistance2();
    
    if (distance >= 2 && distance <= 400) {
      if (distance <= SHORT_DISTANCE2) {
        counter += 10;
        Serial.println(">>> ULTRASONIC2: SHORT RANGE! <<<");
        Serial.println("Distance: " + String(distance) + "cm - Added 10");
      }
      Serial.println("Total Counter: " + String(counter));
      Serial.println("------------------------");
      lastUltrasonicTime = currentTime;
    }
  }
}

void processIRSensors() {
  if (gameEnded) return; // Stop processing sensors when game ended
  
  unsigned long currentTime = millis();
  
  // Read both IR sensors every loop (fast response)
  bool sensor1Detected = (digitalRead(irSensorPin1) == LOW);
  bool sensor2Detected = (digitalRead(irSensorPin2) == LOW);
  bool sensor3Detected = (digitalRead(irSensorPin3) == LOW);
  
  // Check if enough time has passed since last IR detection
  if ((sensor1Detected || sensor2Detected) && 
      (currentTime - lastIRDetectionTime) > irDebounceDelay) {
    
    counter += 100;
    lastIRDetectionTime = currentTime;
    
    Serial.println(">>> IR SENSOR DETECTED! <<<");
    if (sensor1Detected && sensor2Detected) {
      Serial.println("Both IR sensors triggered");
    } else if (sensor1Detected) {
      Serial.println("IR Sensor 1 (Pin 9) triggered");
    } else {
      Serial.println("IR Sensor 2 (Pin 10) triggered");
    }
    Serial.println("Added: 100 points");
    Serial.println("Total Counter: " + String(counter));
    Serial.println("------------------------");
  } else if (sensor3Detected && (currentTime - lastIRDetectionTime) > irDebounceDelay) {
    counter +=30;
    lastIRDetectionTime = currentTime;

   Serial.println(">>> IR SENSOR DETECTED! <<<");
    if (sensor3Detected) {
      Serial.println("IR Sensor 3 (Pin 13) triggered");
    }
    Serial.println("Added: 30 points");
    Serial.println("Total Counter: " + String(counter));
    Serial.println("------------------------");
  }
}

void loop() {
  // Process both sensor types every loop
  processIRSensors();        // Check IR sensors first (fast)
  processUltrasonicSensor1(); // Then check ultrasonic (slower)
  processUltrasonicSensor2(); // Then check.ultrasonic (slower)

  if (counter > 9998) {
    counter = 0;
  }

  delay(20);
}

// I2C request event - NOW SENDS remaining timer 
void requestEvent() {
  char digitsToMaster[4]{};
  digitsToMaster[0] = ((counter / 1000) % 10);  // Thousands
  digitsToMaster[1] = (counter / 100) % 10;  // Hundreds
  digitsToMaster[2] = (counter / 10) % 10;   // Tens
  digitsToMaster[3] = counter % 10;          // Units

  Wire.write(digitsToMaster, 4); // Send all 4 bytes at the same time
}