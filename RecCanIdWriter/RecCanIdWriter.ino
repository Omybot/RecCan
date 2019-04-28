#include <EEPROM.h>

uint16_t canId = 2;

void setup(){
  EEPROM.put(0, canId);
}

void loop() {
}
