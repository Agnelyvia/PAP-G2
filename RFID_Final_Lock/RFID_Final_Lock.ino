#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal.h>

#define RST_PIN 9          // define the RST pin to pin 10
#define SS_PIN 10          // define the SS pin to pin 10
#define servoPin A2        //define the Servo to pin A2
#define freeLED A5         //define the Green LED to pin A5
#define occupiedLED A4     //define the Red LED to pin A4
#define chargingLED A1     //define the Yellow LED to pin A1
#define Buzzer A3          //define the Buzzer to pin A3

const int doorButton = 2;  //define the button pin to pin 2

const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;  //define the LCD pins

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //create an LCD instance to control



bool lockState = true;                                                                             //define the lockState to true, locked
bool verifyNameOfCard(MFRC522::MIFARE_Key key, byte block, byte len, MFRC522::StatusCode status);  //function to verify the name of the card

Servo lockServo;  //create a Servo instance

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(9600);            // Initialize serial communications with the PC
  SPI.begin();                   // Init SPI bus
  lcd.begin(16, 2);              // Begin the LCD with 16 colums and 2 rows
  mfrc522.PCD_Init();            // Init MFRC522 card
  pinMode(servoPin, OUTPUT);     //define the Servo as an  output
  pinMode(freeLED, OUTPUT);      //define the green LED as an output
  pinMode(occupiedLED, OUTPUT);  //define the red LED as an output
  pinMode(chargingLED, OUTPUT);  //define the yellow LED as an output
  pinMode(Buzzer, OUTPUT);       //define the Buzzer as an output
  pinMode(doorButton, INPUT);    //define the button as an input

  lockServo.write(90);  //start the servo in the 90º position, locked

  digitalWrite(freeLED, HIGH);  //turn on the green LED
  lockServo.attach(servoPin);   //attach a Servo to a pin
  lcd.print("pass the card");   //write on the LCD to "pass the card"
}

void loop() {
  int stateButton = digitalRead(doorButton);

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // check if there are any new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // read the card if there is a card
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  Serial.println(F("**Card Detected:**"));




  if (verifyNameOfCard(key, block, len, status) && (stateButton == 1) && (lockState == true)) {          //if the card is correct, the button is pressed and the Servo is in the locked possition
    openDoor();                                                                                          //call the openDoor fucntion
  } else if (verifyNameOfCard(key, block, len, status) && (stateButton == 1) && (lockState == false)) {  //if the card is correct, the button is pressed and the Servo is in the unlocked possition
    closeDoor();                                                                                         //call the closeDoor fucntion
  } else if (verifyNameOfCard(key, block, len, status) && (stateButton == 0) && (lockState == false)) {  //if the card is correct, the button is not pressed and the Servo is in the unlocked possition
    doorNotClosed();                                                                                     //call the doorNotClosed fucntion
  } else {                                                                                               //in every other case where none of those apply
    cardDenied();                                                                                        //call the cardDenied fucntion
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}



bool verifyNameOfCard(MFRC522::MIFARE_Key key, byte block, byte len, MFRC522::StatusCode status) {  //function to verify the name of the card
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

  if ((Nome == "Silva") || (Nome == "Master")) {  //if the name of the card is the same as the name defiened
    return true;   //return that it's correct
  } else {
    return false;  //return that it's inncorrect
  }
}

void openDoor() {
  lcd.clear();                     //clear the LCD of the previous message
  digitalWrite(occupiedLED, LOW);  //turn off the occupied LED
  digitalWrite(freeLED, HIGH);     //turn on the free LED
  digitalWrite(chargingLED, LOW);  //turn off the charging LED
  lcd.print("Door opening");       //print on the LCD "door opening"
  tone(Buzzer, 1000, 1000);        //buzzer makes a sound
  delay(1000);                     //delay the code for 1000 milliseconds (1 second)
  lockServo.write(0);              //turn the servo to the 0º position, unlocked
  lcd.clear();                     //clear the LCD of the previous message
  lcd.print("pass the card");      //print on the LCD "pass the card"
  lockState = false;               //change the lockState to false, meaning the locker is now unlocked
}

void closeDoor() {
  lcd.clear();                      //clear the LCD of the previous message
  digitalWrite(occupiedLED, HIGH);  //turn on the occupied LED
  digitalWrite(freeLED, LOW);       //turn off the free LED
  digitalWrite(chargingLED, HIGH);  //turn on the charging LED
  lcd.print("Door closing");        //write on the LCD "door closing"
  tone(Buzzer, 1000, 1000);         //make the buzzer make a soound
  delay(1000);                      //delay the code for 1000 milliseconds (1 second)
  lockServo.write(90);              //turn the servo to the 90º position, locked
  lockState = true;                 //change the lockState to false, meaning the locker is now locked
  lcd.clear();                      //clear the LCD of the previous message
  lcd.print("In use");       //write on the LCD "pass the card"
}

void doorNotClosed() {
  lcd.clear();                  //clear the LCD of the previous message
  lcd.print("Close the door");  //write on the LCD "close the door"
  tone(Buzzer, 500, 1000);      //make the buzzer make a soound
  delay(1000);                  //delay the code for 1000 milliseconds (1 second)
  tone(Buzzer, 500, 1000);      //make the buzzer make a soound
  delay(1000);                  //delay the code for 1000 milliseconds (1 second)
  lcd.clear();                  //clear the LCD of the previous message
  lcd.print("pass the card");   //write on the LCD "pass the card"
}

void cardDenied() {
  lcd.clear();                   //clear the LCD of the previous message
  lcd.print("card was denied");  //write on the LCD "card was denied"
  tone(Buzzer, 1000, 200);       //make the buzzer make a soound
  delay(200);                    //delay the code for 200 milliseconds
  tone(Buzzer, 1000, 200);       //make the buzzer make a soound
  delay(200);                    //delay the code for 200 milliseconds
  tone(Buzzer, 200, 200);        //make the buzzer make a soound
  delay(200);                    //delay the code for 200 milliseconds
  lcd.clear();                   //clear the LCD of the previous message
  lcd.print("pass the card");    //write on the LCD "pass the card"
}