// These were part of the example and is probably importaint.
#include <Wire.h> 
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
  
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

// Define pins
#define START_PIN 10
#define IGNITION_PIN 8
#define LOCK_PIN 6
#define UNLOCK_PIN 5
#define CANBUS_PIN 2 // Probably unneccesary now that i think of it but ill leave it in case i need another relay later.
#define BUTTON_INPUT 12
#define SERVICE_MODE false // Allows a 'key on but engine not started' state that my mechanic needs.
#define RELOCK_TIMEOUT 3000

void setup() {
  Serial.begin(115200); // Use for debug and testing.
  Serial.println("Hello!");

  // Set pinmodes.
  pinMode(START_PIN, OUTPUT);
  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(CANBUS_PIN, OUTPUT);
  pinMode(UNLOCK_PIN, OUTPUT);
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(BUTTON_INPUT, INPUT_PULLUP); // Button sense (high when not pressed).
  //pinMode(11, OUTPUT); // Button led maybe later.
 
  Serial.println("Pins set");

  nfc.begin(); //im assuming this is importaint

  // Part example code i borrowed from. Checks for a pn534 board connected.
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF); 
  
  // Configure board to read RFID tags
  nfc.SAMConfig(); 
    
  Serial.println("Waiting for an ISO14443A card");
}

void loop() {
  // put your main code here, to run repeatedly:
  boolean tagRead; //Has a card been found 
  boolean serviceMode = SERVICE_MODE;    
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // UPDATE THESE BEFORE TEST
  uint8_t implantUid[] = {};  // Fill with hex uid of main rfid tag
  uint8_t whiteKeyUid[] = {};  //self explanitory
  uint8_t blueKeyUid[] = {};  //self explanitory
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  tagRead = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength); //tell board to look for a target and put the id in the specified variable
  
  if (tagRead) { 
    boolean validImplant = true; //valid starts as true and when checked will either remain true or turn false upon finding and discrepancy
    boolean validBlue = true;
    boolean validWhite = true;
    Serial.println("Found a card!"); 
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < 7; i++) 
    {
      Serial.print(" 0x");
      Serial.print(uid[i], HEX); 
      if (uid[i] != implantUid[i]) { //check each digit of the uid against the valid tags and flag if not equal
        validImplant = false;
      }
      if (uid[i] != whiteKeyUid[i]) {
        validWhite = false;
      }
      if (uid[i] != blueKeyUid[i]) {
        validBlue = false;
      }
    }
    Serial.println("");
    Serial.print("Registered tag: ");
    if (validImplant || validBlue || validWhite) {
      Serial.println("yes");
      digitalWrite(UNLOCK_PIN, HIGH); //unlock
      delay(500);
      digitalWrite(UNLOCK_PIN, LOW);

      int timer = 0;
      while(timer < RELOCK_TIMEOUT && digitalRead(BUTTON_INPUT) == HIGH) {//wait for timer or for start button
        delay(250);
        timer += 25;
      }

      if (timer < RELOCK_TIMEOUT){//if start was pushed before timer hit
        if (serviceMode){
          digitalWrite(CANBUS_PIN, HIGH);//turn on but dont start (mechanic uses this mode)
          delay(750);
          if (digitalRead(BUTTON_INPUT) != HIGH){//if button is held, start
            digitalWrite(IGNITION_PIN, HIGH);
            digitalWrite(START_PIN, HIGH);
            timer = 0;
            while (digitalRead(BUTTON_INPUT) != HIGH && timer < 500){//start fully while held down
            delay(100);
            timer += 10;
            }
            digitalWrite(START_PIN, LOW);
            delay(2000);
            }
          while (digitalRead(BUTTON_INPUT) == HIGH){//wait for button
              delay(250);
            }
            digitalWrite(IGNITION_PIN, LOW);//shut off
            digitalWrite(CANBUS_PIN, LOW);
        }else{//non service mode
          //start the car
         timer = 0;
         digitalWrite(CANBUS_PIN, HIGH);
         digitalWrite(IGNITION_PIN, HIGH);
         digitalWrite(START_PIN, HIGH);
         delay(750); // start for at least 0.75 sec
         while(timer < 300 && digitalRead(BUTTON_INPUT) != HIGH) {//continue starting while the button is held up to 3 extra seconds
          timer += 10;
          delay(100);
         }
         digitalWrite(START_PIN, LOW);//stop starting
         delay(2000);
         while (digitalRead(BUTTON_INPUT) == HIGH){//wait untill button is pressed again
          delay(250);
         }
        digitalWrite(IGNITION_PIN, LOW);//ignition off
        digitalWrite(CANBUS_PIN, LOW);
        //car off
        }
        timer =0;
        while(timer < RELOCK_TIMEOUT) {// wait 30 seconds
         delay(1000);
         timer += 100;
        }
        digitalWrite(LOCK_PIN, HIGH);// lock
        delay(500);
        digitalWrite(LOCK_PIN, LOW);
        
      }else {// if 90 secs pass before button press
        digitalWrite(LOCK_PIN, HIGH);//lock
        delay(500);
        digitalWrite(LOCK_PIN, LOW);
      }
    }else {//if not valid key
      Serial.println("no");
    }
    delay(1000);
  }
  else//no tag found
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out, retrying after delay");
    delay(5000);
  }
}
