#include <EEPROM.h>
#include "Can.h"
#include <MyServoHandler.h>
#include <MyServo.h>

uint16_t boardId;                                     // Identifiant CAN de la carte

unsigned int cptInPackets, cptOutPackets;

unsigned long lastPacketDate;
unsigned long packetTimeout = 100;                    // Temps max (ms) d'essai d'envoi de message

canPacket response;

// Gestion des servos
MyServoHandler servos;
ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

void setup(){
  
  // Gestion des pins
  servos.attach();
  
  EEPROM.get(0, boardId);                             // Récupération id de la carte

  CAN.begin(CAN_125KBPS, MCP_8MHz);                   // Initialisation controleur CAN
  CAN.initCANMasksAndFilters();                       // Configuration des filtres pour n'accepter que id = 0
  CAN.setFilterId( 0, boardId );                      // Ajout aux filtre de l'id de la carte

  response.id = boardId;                              // Les packet CAN que la carte enverra auront l'id de la carte

}

void loop(){

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
          unsigned int position = servos.getPosition(servoId);
          response.msg[2] = position >> 8;
          response.msg[3] = position & 0xFF;
        break;
      }
      
      case PositionSet : {
          unsigned int newPosition = p.msg[2] * 0x100 + p.msg[3];
          servos.setPosition(servoId, newPosition);
          break;
      }
      
      case PositionMinAsk : {
          response.msg[0] = PositionMinResponse;
          response.msg[1] = servoId;
          unsigned int positionMin = servos.getPositionMin(servoId);
          response.msg[2] = positionMin >> 8;
          response.msg[3] = positionMin & 0xFF;
        break;
      }
      
      case PositionMinSet : {
          unsigned int newPositionMin = p.msg[2] * 0x100 + p.msg[3];
          servos.setPositionMin(servoId, newPositionMin);
          break;
      }
      
      case PositionMaxAsk : {
          response.msg[0] = PositionMaxResponse;
          response.msg[1] = servoId;
          unsigned int positionMax = servos.getPositionMax(servoId);
          response.msg[2] = positionMax >> 8;
          response.msg[3] = positionMax & 0xFF;
        break;
      }
      
      case PositionMaxSet : {
          unsigned int newPositionMax = p.msg[2] * 0x100 + p.msg[3];
          servos.setPositionMax(servoId, newPositionMax);
          break;
      }
      
      case SpeedLimitAsk : {
          response.msg[0] = SpeedLimitResponse;
          response.msg[1] = servoId;
          unsigned int speedLimit = servos.getSpeedLimit(servoId);
          response.msg[2] = speedLimit >> 8;
          response.msg[3] = speedLimit & 0xFF;
        break;
      }
      
      case SpeedLimitSet : {
          unsigned int newSpeedLimit = p.msg[2] * 0x100 + p.msg[3];
          servos.setSpeedLimit(servoId, newSpeedLimit);
          break;
      }
      
      case TorqueLimitAsk : {
          response.msg[0] = TorqueLimitResponse;
          response.msg[1] = servoId;
          unsigned int torqueLimit = servos.getTorqueLimit(servoId);
          response.msg[2] = torqueLimit >> 8;
          response.msg[3] = torqueLimit & 0xFF;
        break;
      }
      
      case TorqueLimitSet : {
          unsigned int newTorqueLimit = p.msg[2] * 0x100 + p.msg[3];
          servos.setTorqueLimit(servoId, newTorqueLimit);
          break;
      }
      
      case TorqueAsk : {
          response.msg[0] = TorqueResponse;
          response.msg[1] = servoId;
          unsigned int torque = servos.getTorque(servoId);
          response.msg[2] = torque >> 8;
          response.msg[3] = torque & 0xFF;
        break;
      }
      
      case AccelerationAsk : {
          response.msg[0] = AccelerationResponse;
          response.msg[1] = servoId;
          unsigned int acceleration = servos.getAcceleration(servoId);
          response.msg[2] = acceleration >> 8;
          response.msg[3] = acceleration & 0xFF;
        break;
      }
      
      case AccelerationSet : {
          unsigned int newAcceleration = p.msg[2] * 0x100 + p.msg[3];
          servos.setAcceleration(servoId, newAcceleration);
          break;
      }
      
      case TargetSet : {
//          position = p.msg[2] * 0x100 + p.msg[3];
          break;
      }
      
      case TrajectorySet : {
          unsigned int newPosition = p.msg[2] * 0x100 + p.msg[3];
          unsigned int newSpeedLimit = p.msg[4] * 0x100 + p.msg[5];
          unsigned int newAcceleration = p.msg[6] * 0x100 + p.msg[7];
          servos.setTrajectory(servoId,newPosition, newSpeedLimit, newAcceleration);
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
