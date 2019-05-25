#include <EEPROM.h>
#include <mcp_can.h>
#include "MyServoHandler.h"
#include "MyServo.h"

const uint16_t boardId = 5;                                                           // Id de la carte

unsigned long time;
unsigned long stepTime = 1000;

// Gestion des servos

MyServoHandler servos;
ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

// Gestion CAN

#define CAN_CS_PIN  10
#define CAN_FRAMESIZE 8

MCP_CAN CAN(CAN_CS_PIN);

// Fonction qui initialise masques et filtres pour le bus CAN
void initCANMasksAndFilters(){

  // Buffer de récéption RXB0 (1 masque et 2 filtres associés)
  CAN.init_Mask( 0, 0, 0xFFFF );          // Par défaut, le masque 1 bloque tout type d'identifiant, seul les filtres peuvent autoriser les identifiants
  CAN.init_Filt( 0, 0, boardId );         // Filtre qui n'autorique que l'id de la carte
  CAN.init_Filt( 1, 0, 0 );               // Filtre qui n'autorique que l'id 0

  // Buffer de récéption RXB1 (1 masque et 4 filtres associés)
  CAN.init_Mask( 1, 0, 0xFFFF );          // Par défaut, le masque 1 bloque tout type d'identifiant, seul les filtres peuvent autoriser les identifiants
  CAN.init_Filt( 2, 0, 0 );               // Filtre qui n'autorique que l'id 0
  CAN.init_Filt( 3, 0, 0 );               // Filtre qui n'autorique que l'id 0
  CAN.init_Filt( 4, 0, 0 );               // Filtre qui n'autorique que l'id 0
  CAN.init_Filt( 5, 0, 0 );               // Filtre qui n'autorique que l'id 0

}

bool state = false;

void setup(){

  servos.attach();

  while( CAN_OK != CAN.begin(CAN_500KBPS) ){
    delay(100);
  }
  initCANMasksAndFilters();

}

void loop(){

  // Affichage compteur de trames
  if( millis() > time + stepTime ){
    time += stepTime;

    state = !state;
    digitalWrite(2, state );

  }


    if( CAN_MSGAVAIL == CAN.checkReceive() ){

      // Récupération message
      unsigned char canMsgSize;
      unsigned char canMsg[8];
      CAN.readMsgBuf(&canMsgSize, canMsg);
      unsigned long canId = CAN.getCanId();

      // Gestion des commandes
      byte command = canMsg[0];
      byte servoId = canMsg[1];

      canMsgSize = 0;                             // Retour à zero pour initier message de retour

      switch( command ){

        case PositionAsk : {  // 0x01
            canMsg[0] = PositionResponse;
            canMsg[1] = servoId;
            unsigned int position = servos.getPosition(servoId);
            canMsg[2] = position >> 8;
            canMsg[3] = position & 0xFF;
            canMsgSize = 4;
          break;
        }

        case PositionSet : {  // 0x03
            unsigned int newPosition = canMsg[2] * 0x100 + canMsg[3];
            servos.setPosition(servoId, newPosition);
          break;
        }

        case PositionMinAsk : { // 0x04
            canMsg[0] = PositionMinResponse;
            canMsg[1] = servoId;
            unsigned int positionMin = servos.getPositionMin(servoId);
            canMsg[2] = positionMin >> 8;
            canMsg[3] = positionMin & 0xFF;
            canMsgSize = 4;
          break;
        }

        case PositionMinSet : {  // 0x06
            unsigned int newPositionMin = canMsg[2] * 0x100 + canMsg[3];
            servos.setPositionMin(servoId, newPositionMin);
          break;
        }

        case PositionMaxAsk : {  // 0x07
            canMsg[0] = PositionMaxResponse;
            canMsg[1] = servoId;
            unsigned int positionMax = servos.getPositionMax(servoId);
            canMsg[2] = positionMax >> 8;
            canMsg[3] = positionMax & 0xFF;
            canMsgSize = 4;
          break;
        }

        case PositionMaxSet : {  // 0x09
            unsigned int newPositionMax = canMsg[2] * 0x100 + canMsg[3];
            servos.setPositionMax(servoId, newPositionMax);
          break;
        }

        case SpeedLimitAsk : {  // 0x0A
            canMsg[0] = SpeedLimitResponse;
            canMsg[1] = servoId;
            unsigned int speedLimit = servos.getSpeedLimit(servoId);
            canMsg[2] = speedLimit >> 8;
            canMsg[3] = speedLimit & 0xFF;
            canMsgSize = 4;
          break;
        }

        case SpeedLimitSet : {  // 0x0C
            unsigned int newSpeedLimit = canMsg[2] * 0x100 + canMsg[3];
            servos.setSpeedLimit(servoId, newSpeedLimit);
          break;
        }

        case TorqueLimitAsk : {
            canMsg[0] = TorqueLimitResponse;
            canMsg[1] = servoId;
            unsigned int torqueLimit = servos.getTorqueLimit(servoId);
            canMsg[2] = torqueLimit >> 8;
            canMsg[3] = torqueLimit & 0xFF;
            canMsgSize = 4;
          break;
        }

        case TorqueLimitSet : {  // 0x0F
            unsigned int newTorqueLimit = canMsg[2] * 0x100 + canMsg[3];
            servos.setTorqueLimit(servoId, newTorqueLimit);
          break;
        }

        case TorqueAsk : {  // 0x10
            canMsg[0] = TorqueResponse;
            canMsg[1] = servoId;
            unsigned int torque = servos.getTorque(servoId);
            canMsg[2] = torque >> 8;
            canMsg[3] = torque & 0xFF;
            canMsgSize = 4;
          break;
        }

        case AccelerationAsk : {  // 0x12
            canMsg[0] = AccelerationResponse;
            canMsg[1] = servoId;
            unsigned int acceleration = servos.getAcceleration(servoId);
            canMsg[2] = acceleration >> 8;
            canMsg[3] = acceleration & 0xFF;
            canMsgSize = 4;
          break;
        }

        case AccelerationSet : {  // 0x14
            unsigned int newAcceleration = canMsg[2] * 0x100 + canMsg[3];
            servos.setAcceleration(servoId, newAcceleration);
          break;
        }

        case TargetSet : {  // 0x15
          break;
        }

        case TrajectorySet : {  // 0x16
            unsigned int newPosition = canMsg[2] * 0x100 + canMsg[3];
            unsigned int newSpeedLimit = canMsg[4] * 0x100 + canMsg[5];
            unsigned int newAcceleration = canMsg[6] * 0x100 + canMsg[7];
            servos.setTrajectory(servoId,newPosition, newSpeedLimit, newAcceleration);
          break;
        }

       case DisableOutput : {  // 0x17
           servos.disableOutput(servoId);
         break;
       }

//        case Debug : {
//            break;
//        }

//        case DebugAsk : {
//            canMsg[0] = DebugResponse;
//            canMsg[1] = servoId;
//            canMsg[2] = cptCanIn >> 8;
//            canMsg[3] = cptCanIn & 0xFF;
//            canMsg[4] = cptCanOut >> 8;
//            canMsg[5] = cptCanOut & 0xFF;
//            canMsgSize = 4;
//            break;
//        }

      }

      // Test si réponse à envoyer
      if( canMsgSize > 0 ){

        // Envoi réponse
        CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0);

      }

    }

}
