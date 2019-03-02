#include <EEPROM.h>

uint16_t canId;

void setup(){
  Serial.begin(500000);
  
  EEPROM.get(0, canId);

  Serial.println( canId );
  
}

void loop() {
}
