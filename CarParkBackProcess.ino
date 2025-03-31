#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 20, 4); // Check I2C address with scanner if this doesn't work

// Keypad setup
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo setup
Servo elevatorServo;
Servo rotateServo;
Servo rackServo;

// Parking system variables
int currentFloor = 0;
int parkingSlots[3][6];  // 3 floors, 6 slots per floor
String accessCodes[3][6];  // To store access codes

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);
  lcd.backlight();

  elevatorServo.attach(10);  // Connect to PWM pin 10
  rotateServo.attach(11);    // Connect to PWM pin 11
  rackServo.attach(12);      // Connect to PWM pin 12

  randomSeed(analogRead(0));  // Seed for random code generation

  // Initialize parking slots to empty
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 6; j++) {
      parkingSlots[i][j] = 0;  // 0 means slot is empty
      accessCodes[i][j] = ""; // Empty code
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to Parking");
  delay(2000);
}

void loop() {
  showMainMenu();
}

void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("* Park  # Retrieve");

  char key = keypad.getKey();
  if (key == '*') {
    parkCar();
  } else if (key == '#') {
    retrieveCar();
  }
}

void parkCar() {
  lcd.clear();
  lcd.print("Finding slot...");
  delay(1000);

  int floor = -1, slot = -1;
  bool found = false;

  for (int i = 0; i < 3 && !found; i++) {
    for (int j = 0; j < 6; j++) {
      if (parkingSlots[i][j] == 0) {  // Check for empty slot
        floor = i;
        slot = j;
        found = true;
        break;
      }
    }
  }

  if (found) {
    moveToFloor(floor);
    rotateToSlot(slot);
    moveRack(true);  // Move rack to park the car
    parkingSlots[floor][slot] = 1;  // Mark slot as occupied
    generateRandomCode(floor, slot);
  } else {
    lcd.clear();
    lcd.print("Parking Full!");
    delay(2000);
  }
}

void retrieveCar() {
  lcd.clear();
  lcd.print("Enter Slot:");
  int slotNumber = getNumberInput();

  lcd.clear();
  lcd.print("Enter Code:");
  String enteredCode = getCodeInput();

  int floor = (slotNumber - 1) / 6;
  int slot = (slotNumber - 1) % 6;

  if (parkingSlots[floor][slot] == 1 && accessCodes[floor][slot] == enteredCode) {
    moveToFloor(floor);
    rotateToSlot(slot);
    moveRack(false);  // Move rack to retrieve car
    parkingSlots[floor][slot] = 0;  // Mark slot as empty
    accessCodes[floor][slot] = ""; // Clear access code

    lcd.clear();
    lcd.print("Car Retrieved!");
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("Invalid Slot/Code!");
    delay(2000);
  }
}

void moveToFloor(int floor) {
  int angle = floor * 60;  // Example angles: 0, 60, 120 degrees
  elevatorServo.write(angle);
  delay(2000);  // Adjust based on actual hardware response
}

void rotateToSlot(int slot) {
  int angle = slot * 60;  // Example angles: 0, 60, 120, etc.
  rotateServo.write(angle);
  delay(2000);
}

void moveRack(bool park) {
  if (park) {
    rackServo.write(90);  // Move forward to park
  } else {
    rackServo.write(0);   // Move backward to retrieve
  }
  delay(2000);
}

void generateRandomCode(int floor, int slot) {
  int code = random(1000, 9999);
  accessCodes[floor][slot] = String(code);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Slot: ");
  lcd.print(floor * 6 + slot + 1);
  lcd.setCursor(0, 1);
  lcd.print("Code: ");
  lcd.print(code);
  lcd.setCursor(0, 3);
  lcd.print("* - OK  # - Cancel");

  // Wait for user confirmation
  while (true) {
    char key = keypad.getKey();
    if (key == '*') break;
    if (key == '#') {
      parkingSlots[floor][slot] = 0;  // Cancel parking
      accessCodes[floor][slot] = "";
      lcd.clear();
      lcd.print("Parking Cancelled");
      delay(2000);
      return;
    }
  }
}

int getNumberInput() {
  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key && isDigit(key)) {
      input += key;
      lcd.print(key);
    } else if (key == '*') {
      return input.toInt();
    }
  }
}

String getCodeInput() {
  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key && isDigit(key)) {
      input += key;
      lcd.print('*');  // Mask input for security
    } else if (key == '*') {
      return input;
    }
  }
}
