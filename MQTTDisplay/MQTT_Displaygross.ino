#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
}

void loop() {
  // put your main code here, to run repeatedly:
  tft.setCursor(0,0,2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.println("Hello world!");
  tft.println("Hallo Welt!");
  tft.println("Herzlich Willkommen beim Hackthon!");
}
