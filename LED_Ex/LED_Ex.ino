#include <IRremoteESP8266.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
const uint16_t kRecvPin = 13;

IRrecv irrecv(kRecvPin);

decode_results results;

int R = 5;
int G = 4;
int B = 0;

bool ran = 0;
bool gan = 0;
bool ban = 0;

bool blinking = 0;

int eins = 2;
int zwei = 14;
int drei = 12;

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  // put your setup code here, to run once:
  pinMode(R, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(eins, INPUT_PULLUP);
  pinMode(zwei, INPUT_PULLUP);
  pinMode(drei, INPUT_PULLUP);
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
  
}

void loop() {
  writeLEDs();
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // Receive the next value
    if(results.value == 0xa90){
      blinking = !blinking;
    }
  }
  
  if(blinking){
    ran = 0;
    gan = 0;
    if(ban == 0){
        ban = 1;
      } else {
        ban = 0;
      }
  } else {
    int value = 0;
    value = digitalRead(zwei);
    if(value == 0){
      if(ran == 0){
        ran = 1;
      } else {
        ran = 0;
      }
    }
    value = digitalRead(eins);
    if(value == 0){
      if(gan == 0){
        gan = 1;
      } else {
        gan = 0;
      }
    }
    value = digitalRead(drei);
    if(value == 0){
      if(ban == 0){
        ban = 1;
      } else {
        ban = 0;
      }
    }
  }
  
  delay(500);
  //Serial.println("Hello world");
}

void writeLEDs(){
  digitalWrite(R, ran);
  digitalWrite(G, gan);
  digitalWrite(B, ban);
}
