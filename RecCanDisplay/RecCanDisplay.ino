#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup(){

  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

  display.setTextSize(7);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.clearDisplay();
  display.println("123");
  display.display();

}


void loop() {

}
