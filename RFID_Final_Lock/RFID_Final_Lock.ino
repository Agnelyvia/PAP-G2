#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal.h>

#define RST_PIN 9  // Configurable, see typical pin layout above
#define SS_PIN 10  // Configurable, see typical pin layout above
#define servoPin A2
#define freeLED A5
#define occupiedLED A4
#define Buzzer A3

const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int doorButton = 2;



bool lockState = true;
bool verifyNameOfCard(MFRC522::MIFARE_Key key, byte block, byte len, MFRC522::StatusCode status);

Servo lockServo;



MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void openDoor();
void closeDoor();
void doorNotClosed();
void cardDenied();

void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC
  SPI.begin();         // Init SPI bus
  lcd.begin(16, 2);
  mfrc522.PCD_Init();  // Init MFRC522 card
  pinMode(servoPin, OUTPUT);
  pinMode(freeLED, OUTPUT);
  pinMode(occupiedLED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(doorButton, INPUT);

  lockServo.write(90);

  digitalWrite(freeLED, HIGH);
  lockServo.attach(servoPin);
  lcd.print("pass the card");
}

void loop() {
  int stateButton = digitalRead(doorButton);
  Serial.println(stateButton);
  delay(500);

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  Serial.println(F("**Card Detected:**"));




  if (verifyNameOfCard(key, block, len, status) && (stateButton == 1) && (lockState == true)) {
    openDoor();
  }
  else if (verifyNameOfCard(key, block, len, status) && (stateButton == 1) && (lockState == false)) {
    closeDoor();
  }
  else if (verifyNameOfCard(key, block, len, status) && (stateButton == 0) && (lockState == false)) {
    doorNotClosed();
  }
  else {
    cardDenied();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}



bool verifyNameOfCard(MFRC522::MIFARE_Key key, byte block, byte len, MFRC522::StatusCode status) {
  Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));  //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  String Nome;
  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++) {
    if (buffer1[i] != 32) {
      Serial.write(buffer1[i]);
      Nome += String((char)buffer1[i]);
    }
  }
  Serial.print(" ");
  Nome.trim();

  Serial.println(F("**End Reading**"));

  if ((Nome == "Silva") || (Nome == "Master")) {
    return true;
  } else {
    return false;
  }
}

void openDoor() {
  lcd.clear();
  digitalWrite(occupiedLED, LOW);
  digitalWrite(freeLED, HIGH);
  lcd.print("Door opening");
  tone(Buzzer, 1000, 1000);
  delay(1000);
  lockServo.write(0);
  lcd.clear();
  lcd.print("pass the card");
  lockState = false;
}

void closeDoor() {
  lcd.clear();
  digitalWrite(occupiedLED, HIGH);
  digitalWrite(freeLED, LOW);
  lcd.print("Door closing");
  tone(Buzzer, 1000, 1000);
  delay(1000);
  lockServo.write(90);
  lockState = true;
  lcd.clear();
  lcd.print("pass the card");
}

void doorNotClosed() {
  lcd.clear();
  lcd.print("Close the door");
  tone(Buzzer, 500, 1000);
  delay(1000);
  tone(Buzzer, 500, 1000);
  lcd.clear();
  lcd.print("pass the card");
}

void cardDenied() {
  lcd.clear();
  lcd.print("card was denied");
  tone(Buzzer, 1000, 200);
  delay(200);
  tone(Buzzer, 1000, 200);
  delay(200);
  tone(Buzzer, 200, 200);
  delay(500);
  lcd.clear();
  lcd.print("pass the card");
}