#include <Adafruit_NeoPixel.h>

#define PIN 12

Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  strip.begin();
  strip.setBrightness(90);
  strip.show();

  delay(2000);
  showRainbow();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Set colours");
 
  delay(10000);
}

void showRainbow(){
  strip.setPixelColor(0, 204, 0, 153);
  strip.setPixelColor(1, 255, 0, 0);
  strip.setPixelColor(2, 255, 102, 0);
  strip.setPixelColor(3, 255, 204, 0);
  strip.setPixelColor(4, 102, 153, 0);
  strip.setPixelColor(5, 0, 204, 102);
  strip.setPixelColor(6, 0, 51, 204);
  strip.setPixelColor(7, 153, 0, 255);
  
  strip.show();
}
