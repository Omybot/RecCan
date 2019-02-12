#include <EEPROM.h>
#include "Can.h"

uint16_t boardId;                                      // Identifiant CAN de la carte

unsigned int pos;                                      // Simulation position du servo

void setup(){

  Serial.begin( 500000 );
  
  EEPROM.get(0, boardId);                             // Récupération id de la carte

  CAN.begin(CAN_80KBPS, MCP_8MHz);                    // Initialisation controleur CAN
  CAN.initCANMasksAndFilters();                       // Configuration des filtres pour n'accepter que id = 0
  CAN.setFilterId( 0, boardId );                      // Ajout aux filtre de l'id de la carte

}

void loop(){
  
  while( CAN.checkNewPacket() ){                      // Tant que packet CAN à lire
    canPacket p = CAN.getNewPacket();                 // Récupération du packet CAN

    byte command = p.msg[0];
    byte servoId = p.msg[1];

    Serial.println("Nouvelle Reception :");
    Serial.print( "Command N°" );
    Serial.print( command );
    Serial.print( ", ServoID N°" );
    Serial.println( servoId );

    switch( command ){

      case 1 : {                                      // PositionAsk
          p.msg[0] = 2;                               // Retour avec la commande Position Response
          p.msg[2] = pos >> 8;
          p.msg[3] = pos & 0xFF;
          CAN.sendPacket( p );                        // Envoi du packet de retour
        break;
      }
      
      case 3 : {                                      // SetPosition
          pos = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
    }
    
  }

}
