#include <EEPROM.h>

void setup() {
  byte canId = 1;
  EEPROM.write(0, canId);
}

void loop() {
}
