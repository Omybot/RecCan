#include <EEPROM.h>
#include "Can.h"

uint16_t boardId;                                     // Identifiant CAN de la carte

unsigned int cptInPackets, cptOutPackets;

unsigned long lastPacketDate;
unsigned long packetTimeout = 100;                    // Temps max (ms) d'essai d'envoi de message

canPacket response;

// Simulation variables servo
unsigned int position, positionMin, positionMax;                                
unsigned int speed;
unsigned int torqueCurrent, torqueMax;
unsigned int acceleration;

unsigned long time, stepTime = 500;

void setup(){

  Serial.begin(500000);
  
  EEPROM.get(0, boardId);                             // Récupération id de la carte

  CAN.begin(CAN_125KBPS, MCP_8MHz);                   // Initialisation controleur CAN
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
    } else if( lastPacketDate > millis() + packetTimeout ){
      for( int i=0; i<8; i++ ) response.msg[i] = 0x00;   // Réinitialisation message reponse
    }
  }

  // Récéption depuis le bus CAN
  if( CAN.checkNewPacket() ){                         // Tant que packet CAN à lire
    lastPacketDate = millis();
    cptInPackets++;
    canPacket p = CAN.getNewPacket();                 // Récupération du packet CAN

    byte command = p.msg[0];
    byte servoId = p.msg[1];

    switch( command ){

      case PositionAsk : {
          response.msg[0] = PositionResponse;
          response.msg[1] = servoId;
          response.msg[2] = position >> 8;
          response.msg[3] = position & 0xFF;
        break;
      }
      
      case PositionSet : {
          position = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case PositionMinAsk : {
          response.msg[0] = PositionMinResponse;
          response.msg[1] = servoId;
          response.msg[2] = positionMin >> 8;
          response.msg[3] = positionMin & 0xFF;
        break;
      }
      
      case PositionMinSet : {
          positionMin = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case PositionMaxAsk : {
          response.msg[0] = PositionMaxResponse;
          response.msg[1] = servoId;
          response.msg[2] = positionMax >> 8;
          response.msg[3] = positionMax & 0xFF;
        break;
      }
      
      case PositionMaxSet : {
          positionMax = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case SpeedAsk : {
          response.msg[0] = SpeedResponse;
          response.msg[1] = servoId;
          response.msg[2] = speed >> 8;
          response.msg[3] = speed & 0xFF;
        break;
      }
      
      case SpeedSet : {
          speed = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case TorqueMaxAsk : {
          response.msg[0] = TorqueMaxResponse;
          response.msg[1] = servoId;
          response.msg[2] = torqueMax >> 8;
          response.msg[3] = torqueMax & 0xFF;
        break;
      }
      
      case TorqueMaxSet : {
          torqueMax = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case TorqueCurrentAsk : {
          response.msg[0] = TorqueCurrentResponse;
          response.msg[1] = servoId;
          response.msg[2] = torqueCurrent >> 8;
          response.msg[3] = torqueCurrent & 0xFF;
        break;
      }
      
      case AccelerationAsk : {
          response.msg[0] = AccelerationResponse;
          response.msg[1] = servoId;
          response.msg[2] = acceleration >> 8;
          response.msg[3] = acceleration & 0xFF;
        break;
      }
      
      case AccelerationSet : {
          acceleration = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case TargetSet : {
          position = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case TrajectorySet : {
          position = p.msg[2] * 0x100 + p.msg[3];
          speed = p.msg[4] * 0x100 + p.msg[5];
          acceleration = p.msg[6] * 0x100 + p.msg[7];
          break;
      }

      case Debug : {
          break;
      }
      
      case DebugAsk : {
          response.msg[0] = DebugResponse;
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
