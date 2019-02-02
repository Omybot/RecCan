#include <EEPROM.h>
#include "Can.h"

uint16_t boardId;                                      // Identifiant CAN de la carte

unsigned int pos = 12;

void setup(){

  Serial.begin( 500000 );

  EEPROM.get(0, boardId);

  CAN.begin(CAN_80KBPS, MCP_8MHz);                    // Initialisation controleur CAN
  CAN.initCANMasksAndFilters();                       // Configuration des filtres pour n'accepter que id = 0
  CAN.setFilterId( 0, boardId );                      // Ajout aux filtre de l'id de la carte

}

void loop(){
  
  while( CAN.checkNewPacket() ){                      // Tant que packet CAN à lire
    canPacket p = CAN.getNewPacket();                 // Récupération du packet CAN

    byte command = p.msg[0];
    byte servoId = p.msg[1];

    switch( command ){

      // Gestion position
      case 3 : {                                      // SetPosition
          pos = *(int*)&p.msg[2];
      }
      case 1 : {                                      // PositionAsk
          p.id = 2;
          byte *ptr = (byte*)&pos;
          for( int i=0; i<4; i++ ) p.msg[i+2] = ptr[i];
        break;
      }
      
    }
    
    CAN.sendPacket( p );                              // Renvoi du packet recu
    
  }

}
