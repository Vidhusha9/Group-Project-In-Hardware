#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Initialize the I2C 20x4 LCD (I2C address 0x27, 20 characters, 4 rows)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Initialize Servo motors
Servo servoEntrance;
Servo servoExit;

// IR sensor pins
const int irSensor1 = 6;  // Entrance sensor
const int irSensor2 = 7;  // After entrance sensor
const int irSensor3 = 8;  // Exit sensor
const int irSensor4 = 9;  // After exit sensor

// Diode bulb pins
const int bulb1 = 10;  // Bulb 1
const int bulb2 = 11;  // Bulb 2
const int bulb3 = 12;  // Bulb 3
const int bulb4 = 13;  // Bulb 4

int availableSlots = 4;  // Total available slots
int maxSlots = 4;

// Servo positions
int openPosition = 90;
int closePosition = 180;
int exitOpenPosition = 90;
int exitClosePosition = 0;

unsigned long zeroSlotsStartTime = 0;  // Timer to track when slots became 0

void setup() {
  // Initialize the LCD
  lcd.init();
  lcd.backlight();

  // Attach the servo motors to the pins
  servoEntrance.attach(4);
  servoExit.attach(5);

  // Set servo motors to their initial positions
  servoEntrance.write(closePosition);
  servoExit.write(exitClosePosition);

  // Initialize IR sensor pins with internal pull-up resistors
  pinMode(irSensor1, INPUT_PULLUP);
  pinMode(irSensor2, INPUT_PULLUP);
  pinMode(irSensor3, INPUT_PULLUP);
  pinMode(irSensor4, INPUT_PULLUP);

  // Initialize bulb pins
  pinMode(bulb1, OUTPUT);
  pinMode(bulb2, OUTPUT);
  pinMode(bulb3, OUTPUT);
  pinMode(bulb4, OUTPUT);

  digitalWrite(bulb1, LOW);
  digitalWrite(bulb2, LOW);
  digitalWrite(bulb3, LOW);
  digitalWrite(bulb4, LOW);

  // Display welcome message
  lcd.setCursor((20 - strlen("Welcome")) / 2, 1); // Centered on line 1
  lcd.print("Welcome");
  delay(2000);
  lcd.clear();
  lcd.setCursor((20 - strlen("Automated")) / 2, 0); // Centered on line 0
  lcd.print("Automated");
  lcd.setCursor((20 - strlen("Car Parking")) / 2, 1); // Centered on line 1
  lcd.print("Car Parking");
  lcd.setCursor((20 - strlen("System")) / 2, 2); // Centered on line 2
  lcd.print("System");
  lcd.setCursor((20 - strlen("_ _ _ _ _ _ _ _ _ _")) / 2, 3); // Centered on line 3
  lcd.print("_ _ _ _ _ _ _ _ _ _");
  delay(3000);

  // Display initial available slots
  updateSlotsDisplay();
  updateBulbState();
}

void loop() {
  // Read IR sensor states with debounce
  int sensor1State = debounce(irSensor1);
  int sensor2State = debounce(irSensor2);
  int sensor3State = debounce(irSensor3);
  int sensor4State = debounce(irSensor4);

  // Entrance gate logic
  if (sensor1State == LOW && availableSlots > 0) { // Car detected at entrance
    servoEntrance.write(openPosition); // Open entrance gate
    delay(1000); // Wait for car to enter
  }

  if (sensor2State == LOW) { // Car passed the entrance gate
    servoEntrance.write(closePosition); // Close entrance gate
    if (availableSlots > 0) {
      availableSlots--; // Decrease available slots
      updateSlotsDisplay();
      updateBulbState(); // Update bulb state
    }
    delay(1000); // Debouncing
  }

  // Exit gate logic
  if (sensor3State == LOW && availableSlots < maxSlots) { // Car detected at exit
    servoExit.write(exitOpenPosition); // Open exit gate
    delay(1000); // Wait for car to exit
  }

  if (sensor4State == LOW) { // Car passed the exit gate
    servoExit.write(exitClosePosition); // Close exit gate
    if (availableSlots < maxSlots) {
      availableSlots++; // Increase available slots
      updateSlotsDisplay();
      updateBulbState(); // Update bulb state
    }
    delay(1000); // Debouncing
  }

  // Update bulb state continuously to handle 10-second logic for bulb4
  updateBulbState();
}

// Function to update available slots on the LCD
void updateSlotsDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Available Slots: ");
  lcd.print(availableSlots);

  lcd.setCursor(0, 2);
  lcd.print("_ _ _ _ _ _ _ _ _ _");
}

// Function to update the bulb states
void updateBulbState() {
  if (availableSlots >= 3) {
    digitalWrite(bulb1, HIGH);
    digitalWrite(bulb2, LOW);
    digitalWrite(bulb3, LOW);
    digitalWrite(bulb4, HIGH);
    zeroSlotsStartTime = 0; // Reset timer
  } else if (availableSlots == 2) {
    digitalWrite(bulb1, HIGH);
    digitalWrite(bulb2, LOW);
    digitalWrite(bulb3, LOW);
    digitalWrite(bulb4, HIGH);
    zeroSlotsStartTime = 0; // Reset timer
  } else if (availableSlots == 1) {
    digitalWrite(bulb1, LOW);
    digitalWrite(bulb2, HIGH);
    digitalWrite(bulb3, HIGH);
    digitalWrite(bulb4, LOW);
    zeroSlotsStartTime = 0; // Reset timer
  } else if (availableSlots == 0) {
    digitalWrite(bulb1, LOW);
    digitalWrite(bulb2, HIGH);
    digitalWrite(bulb3, HIGH);
    // Start timer if not already started
    if (zeroSlotsStartTime == 0) {
      zeroSlotsStartTime = millis();
    }
    // Check if 10 seconds have passed
    if (millis() - zeroSlotsStartTime >= 10000) {
      digitalWrite(bulb3, LOW);
      digitalWrite(bulb4, HIGH); // Turn bulb4 ON after 10 seconds
    }
  }
}

// Debounce function to avoid false triggering
int debounce(int pin) {
  int state = digitalRead(pin);
  delay(50); // 50ms debounce time
  int newState = digitalRead(pin);
  if (state == newState) {
    return state;
  }
  return HIGH; // Default state if bouncing
}
