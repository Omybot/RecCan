#include <EEPROM.h>
#include "Can.h"

uint16_t boardId;                                      // Identifiant CAN de la carte

unsigned int pos;                                      // Simulation position du servo

unsigned int cptInPackets, cptOutPackets;

canPacket response;

unsigned long time, stepTime = 500;

void setup(){

  Serial.begin(500000);
  
  EEPROM.get(0, boardId);                             // Récupération id de la carte

  CAN.begin(CAN_125KBPS, MCP_8MHz);                    // Initialisation controleur CAN
  CAN.initCANMasksAndFilters();                       // Configuration des filtres pour n'accepter que id = 0
  CAN.setFilterId( 0, boardId );                      // Ajout aux filtre de l'id de la carte

  response.id = boardId;                              // Les packet CAN que la carte enverra auront l'id de la carte

}

void loop(){

  if( millis() > time + stepTime ){
    time += stepTime;
    Serial.print( cptInPackets );
    Serial.print( " " );
    Serial.print( cptOutPackets );
    Serial.println();
  }

  // Envoi sur bus CAN
  if( response.msg[0] != 0 ){                         // Si il y a un message à envoyer
    if( CAN.sendPacket( response ) == CAN_OK ){       // Tentative d'envoi
      for( int i=0; i<8; i++ ) response.msg[i] = 0x00;   // Réinitialisation message reponse
      cptOutPackets++;
    }
  }

  // Récéption depuis le bus CAN
  if( CAN.checkNewPacket() ){                         // Tant que packet CAN à lire
    canPacket p = CAN.getNewPacket();                 // Récupération du packet CAN
    cptInPackets++;

    byte command = p.msg[0];
    byte servoId = p.msg[1];

    switch( command ){

      case 1 : {                                      // PositionAsk
          response.msg[0] = 2;                        // => PositionResponse
          response.msg[1] = servoId;
          response.msg[2] = pos >> 8;
          response.msg[3] = pos & 0xFF;
        break;
      }
      
      case 3 : {                                      // SetPosition
          pos = p.msg[2] * 0x100 + p.msg[3];
          break;
      }

      case 0xF0 : {                                   // Debug
          break;
      }
      
      case 0xF1 : {                                   // Debug Ask
          response.msg[0] = 0xF2;                     // => DebugResponse
          response.msg[1] = servoId;
          response.msg[2] = cptInPackets >> 8;
          response.msg[3] = cptInPackets & 0xFF;
          response.msg[4] = cptOutPackets >> 8;
          response.msg[5] = cptOutPackets & 0xFF;
          break;
      }
      
    }
    
  }

}
