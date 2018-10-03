/*
 *  Created by TheCircuit
*/
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 4  //D2
#define RST_PIN 5 //D1

MFRC522 mfrc522_1(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int statuss = 0;
int out = 0;

void setup() {
  Serial.begin(9600);   // Initiate a serial communication
  Serial.println("Starting...");
  SPI.begin();      // Initiate  SPI bus
  mfrc522_1.PCD_Init();   // Initiate MFRC522
}

void loop() {
  // Look for new cards
  if ( ! mfrc522_1.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522_1.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522_1.uid.size; i++) 
  {
     Serial.print(mfrc522_1.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522_1.uid.uidByte[i], HEX);
     content.concat(String(mfrc522_1.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522_1.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  if (content.substring(1) == "AA BB CC DD") //change UID of the card that you want to give access
  {
    Serial.println(" Access Granted ");
    Serial.println(" Welcome Mr.Circuit ");
    delay(1000);
    Serial.println(" Have FUN ");
    Serial.println();
    statuss = 1;
  }
  
  else   {
    Serial.println(" Access Denied ");
    delay(3000);
  }
} 
