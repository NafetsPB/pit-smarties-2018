#include <IRremoteESP8266.h>
#include <IRsend.h>

int taster = 4;
int infra = 5;

IRsend irsend(infra);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  irsend.begin();
  pinMode(taster, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  int value = digitalRead(taster);
  if(value == 0){
    Serial.println("Senden");
    irsend.sendSony(0xa90, 12, 2);
    // senden
  }
  delay(50);
}
