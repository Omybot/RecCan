#include <EEPROM.h>
#include <mcp_can.h>
#include "MyServoHandler.h"
#include "MyServo.h"

#define DEBUG_EN  1

unsigned int cptCanIn, cptCanOut;

#ifdef DEBUG_EN
unsigned long time;
unsigned long stepTime = 1000;
#endif

// Gestion des servos

MyServoHandler servos;
ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

// Gestion CAN

#define CAN_CS_PIN  8
#define CAN_FRAMESIZE 8

MCP_CAN CAN(CAN_CS_PIN); 

unsigned char flagRecv = 0;

void MCP2515_ISR(){
    flagRecv = 1;
}

// Fonction qui initialise masques et filtres pour le bus CAN
void initCANMasksAndFilters(){

  // Récupération boardId  
  uint16_t boardId;
  EEPROM.get(0, boardId);                 // Récupération id de la carte
  #ifdef DEBUG_EN
  Serial.print( "BoardId : " );
  Serial.println( boardId );
  #endif
  
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

#ifdef DEBUG_EN
void printCANFrame( unsigned int canId, const byte *canMsg, unsigned char canMsgSize ){
  
  Serial.print( "ID : 0x");
  if( canId < 1000 ) Serial.print( "0" );
  if( canId < 100 ) Serial.print( "0" );
  if( canId < 10 ) Serial.print( "0" );
  Serial.print( canId, HEX );
  Serial.print( ", MSG (" );
  Serial.print( canMsgSize );
  Serial.print( ") : " );
  for( int i=0 ; i<canMsgSize ; i++ ){
    if( canMsg[i] < 0x10 ) Serial.print( "0" );
    Serial.print( canMsg[i], HEX );
    if( i < canMsgSize-1 ) Serial.print( "-" );
  }
  Serial.println();
  
}
#endif

void setup(){
  
  #ifdef DEBUG_EN
  Serial.begin(500000);
  #endif
    
  servos.attach();

  while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);
  initCANMasksAndFilters();
  attachInterrupt(0, MCP2515_ISR, FALLING);

  #ifdef DEBUG_EN
  Serial.println( "Init OK" );
  #endif
  
}

void loop(){

  #ifdef DEBUG_EN
  // Affichage compteur de trames
  if( millis() > time + stepTime ){
    time += stepTime;

    Serial.print( "\ncptCan In/Out : " );
    Serial.print( cptCanIn );
    Serial.print( " / " );
    Serial.println( cptCanOut );
    
  }
  #endif
  
  // Si reception trame CAN
  if( flagRecv ){
    flagRecv = 0;
    
    while( CAN_MSGAVAIL == CAN.checkReceive() ){

      cptCanIn++;

      // Récupération message
      unsigned char canMsgSize;
      unsigned char canMsg[8];
      CAN.readMsgBuf(&canMsgSize, canMsg);
      unsigned long canId = CAN.getCanId();

      // Affichage trame recue
      #ifdef DEBUG_EN
      Serial.print( "\nNouvelle reception CAN => " );
      printCANFrame( canId, canMsg, canMsgSize );
      #endif

      // Gestion des commandes
      byte command = canMsg[0];
      byte servoId = canMsg[1];

      #ifdef DEBUG_EN
      Serial.print( "Commande N°" );
      Serial.print( command );
      Serial.print( ", servo ID N°" );
      Serial.println( servoId );
      #endif
      
      canMsgSize = 0;                             // Retour à zero pour initier message de retour

      switch( command ){

        case PositionAsk : {  // 0x01
            canMsg[0] = PositionResponse;
            canMsg[1] = servoId;
            unsigned int position = servos.getPosition(servoId);
            canMsg[2] = position >> 8;
            canMsg[3] = position & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "PositionAsk : " );
            Serial.println( position );
            #endif
          break;
        }
      
        case PositionSet : {  // 0x03
            unsigned int newPosition = canMsg[2] * 0x100 + canMsg[3];
            servos.setPosition(servoId, newPosition);
            #ifdef DEBUG_EN
            Serial.print( "PositionSet : " );
            Serial.println( newPosition );
            #endif
          break;
        }
      
        case PositionMinAsk : { // 0x04
            canMsg[0] = PositionMinResponse;
            canMsg[1] = servoId;
            unsigned int positionMin = servos.getPositionMin(servoId);
            canMsg[2] = positionMin >> 8;
            canMsg[3] = positionMin & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "PositionMinAsk : " );
            Serial.println( positionMin );
            #endif
          break;
        }
      
        case PositionMinSet : {  // 0x06
            unsigned int newPositionMin = canMsg[2] * 0x100 + canMsg[3];
            servos.setPositionMin(servoId, newPositionMin);
            #ifdef DEBUG_EN
            Serial.print( "PositionMinSet : " );
            Serial.println( newPositionMin );
            #endif
          break;
        }
      
        case PositionMaxAsk : {  // 0x07
            canMsg[0] = PositionMaxResponse;
            canMsg[1] = servoId;
            unsigned int positionMax = servos.getPositionMax(servoId);
            canMsg[2] = positionMax >> 8;
            canMsg[3] = positionMax & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "PositionMaxAsk : " );
            Serial.println( positionMax );
            #endif
          break;
        }
      
        case PositionMaxSet : {  // 0x09
            unsigned int newPositionMax = canMsg[2] * 0x100 + canMsg[3];
            servos.setPositionMax(servoId, newPositionMax);
            #ifdef DEBUG_EN
            Serial.print( "PositionMaxSet : " );
            Serial.println( newPositionMax );
            #endif
          break;
        }
      
        case SpeedLimitAsk : {  // 0x0A
            canMsg[0] = SpeedLimitResponse;
            canMsg[1] = servoId;
            unsigned int speedLimit = servos.getSpeedLimit(servoId);
            canMsg[2] = speedLimit >> 8;
            canMsg[3] = speedLimit & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "SpeedLimitAsk : " );
            Serial.println( speedLimit );
            #endif
          break;
        }
      
        case SpeedLimitSet : {  // 0x0C
            unsigned int newSpeedLimit = canMsg[2] * 0x100 + canMsg[3];
            servos.setSpeedLimit(servoId, newSpeedLimit);
            #ifdef DEBUG_EN
            Serial.print( "SpeedLimitSet : " );
            Serial.println( newSpeedLimit );
            #endif
          break;
        }
      
        case TorqueLimitAsk : {
            canMsg[0] = TorqueLimitResponse;
            canMsg[1] = servoId;
            unsigned int torqueLimit = servos.getTorqueLimit(servoId);
            canMsg[2] = torqueLimit >> 8;
            canMsg[3] = torqueLimit & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "TorqueLimitAsk : " );
            Serial.println( torqueLimit );
            #endif
          break;
        }
        
        case TorqueLimitSet : {  // 0x0F
            unsigned int newTorqueLimit = canMsg[2] * 0x100 + canMsg[3];
            servos.setTorqueLimit(servoId, newTorqueLimit);
            #ifdef DEBUG_EN
            Serial.print( "TorqueLimitSet : " );
            Serial.println( newTorqueLimit );
            #endif
          break;
        }
      
        case TorqueAsk : {  // 0x10
            canMsg[0] = TorqueResponse;
            canMsg[1] = servoId;
            unsigned int torque = servos.getTorque(servoId);
            canMsg[2] = torque >> 8;
            canMsg[3] = torque & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "TorqueAsk : " );
            Serial.println( torque );
            #endif
          break;
        }
        
        case AccelerationAsk : {  // 0x12
            canMsg[0] = AccelerationResponse;
            canMsg[1] = servoId;
            unsigned int acceleration = servos.getAcceleration(servoId);
            canMsg[2] = acceleration >> 8;
            canMsg[3] = acceleration & 0xFF;
            canMsgSize = 4;
            #ifdef DEBUG_EN
            Serial.print( "AccelerationAsk : " );
            Serial.println( acceleration );
            #endif
          break;
        }
      
        case AccelerationSet : {  // 0x14
            unsigned int newAcceleration = canMsg[2] * 0x100 + canMsg[3];
            servos.setAcceleration(servoId, newAcceleration);
            #ifdef DEBUG_EN
            Serial.print( "AccelerationSet : " );
            Serial.println( newAcceleration );
            #endif
          break;
        }
        
        case TargetSet : {  // 0x15
            #ifdef DEBUG_EN
            Serial.print( "TargetSet !" );
            #endif
          break;
        }
        
        case TrajectorySet : {  // 0x16
            unsigned int newPosition = canMsg[2] * 0x100 + canMsg[3];
            unsigned int newSpeedLimit = canMsg[4] * 0x100 + canMsg[5];
            unsigned int newAcceleration = canMsg[6] * 0x100 + canMsg[7];
            servos.setTrajectory(servoId,newPosition, newSpeedLimit, newAcceleration);
            #ifdef DEBUG_EN
            Serial.print( "TrajectorySet !" );
            #endif
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
        
        // Affichage trame CAN à envoyer
        #ifdef DEBUG_EN
        Serial.print( "\nNouvel envoi CAN => " );
        printCANFrame( canId, canMsg, canMsgSize );
        #endif
        
        // Envoi réponse
        bool sendStatus = CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0);
        if( sendStatus == CAN_OK ){
          cptCanOut++;
        } else {
          #ifdef DEBUG_EN
          Serial.println( "Envoi KO !!!!!" );
          #endif
        }
        
      }

    }
    
  }
  
}
