#include <EEPROM.h>
#include <MyQueue.h>
#include <SPI.h>
#include "mcp_can.h"
#include <MyServoHandler.h>

/////////////////////////////////
// Gestion bus CAN
/////////////////////////////////

#define CAN_ID  5

#define SPI_CS_PIN 8
MCP_CAN CAN(SPI_CS_PIN);

// Déclaration structure pour une trame
struct canFrame{
  unsigned int id;
  unsigned char size;
  unsigned char msg[8];
};

/////////////////////////////////
// Gestion des servos
/////////////////////////////////

MyServoHandler servos;

/////////////////////////////////
// Gestion des interruptions
/////////////////////////////////
ISR(TIMER1_COMPA_vect){ 
  servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ 
  servos.timer2Interrupt(); 
}

/////////////////////////////////
// SETUP
/////////////////////////////////
void setup() {

  //Serial.begin(115200);
  
  // Gestion des pins
  servos.attach();
  
  // Initialisation CAN
  while (CAN_OK != CAN.begin(CAN_500KBPS)){ delay(10); }

  // Filtre pour ne prendre en compte que les trames désirées
  CAN.init_Mask(0, 0, 0x7ff); 
  CAN.init_Mask(1, 0, 0x7ff); 
  CAN.init_Filt(0, 0, CAN_ID);    // Ne prend en compte que les trames d'identifiant CAN_ID

}

/////////////////////////////////
// LOOP
/////////////////////////////////
void loop() {
  
  if( CAN_MSGAVAIL == CAN.checkReceive() ){                             // Test si des trames ont été recues
    
    canFrame f;
    CAN.readMsgBuf( &f.size, f.msg );
    f.id = CAN.getCanId();
    
    byte frameNumber = f.msg[0];
    byte command = f.msg[1];
    byte servoId = f.msg[2];
    
    canFrame response;
    response.id = f.id;
    response.msg[0] = frameNumber;
    for( int i=1; i<7; i++ ) response.msg[i] = f.msg[i];
    response.size = 8;

    unsigned int buf;
    
    switch( command ){

      // Gestion position
      case 0x01:      // Demande position cible
          response.msg[1] = 0x02;           // Commande retour position
          //response.msg[2] = servoId;
          buf = servos.getPosition( servoId );
          response.msg[3] = buf / 0x100;    // [MSB] position
          response.msg[4] = buf % 0x100;    // [LSB] position
        break;
      case 0x03 :     // Envoi position cible
          buf = f.msg[3] * 0x100 + f.msg[4];
          servos.setPosition( servoId, buf );
        break;

//      // Gestion position min
//      case 0x04:      // Demande position min
//          response.msg[1] = 0x05;           // Commande retour position min
//          buf = servos.getMinPosition( servoId );
//          response.msg[3] = buf / 0x100;    // [MSB] position min
//          response.msg[4] = buf % 0x100;    // [LSB] position min
//        break;
//      case 0x06 :     // Envoi position min    
//          buf = f.msg[3] * 0x100 + f.msg[4];
//          servos.setMinPosition( servoId, buf );
//        break;
//
//      // Gestion position max
//      case 0x07:      // Demande position max
//          response.msg[1] = 0x08;           // Commande retour position max
//          buf = servos.getMaxPosition( servoId );
//          response.msg[3] = buf / 0x100;    // [MSB] position max
//          response.msg[4] = buf % 0x100;    // [LSB] position max
//        break;
//      case 0x09 :     // Envoi position max    
//          buf = f.msg[3] * 0x100 + f.msg[4];
//          servos.setMaxPosition( servoId, buf );
//        break;
        
      // Gestion vitesse max
      case 0x0A:      // Demande vitesse max
          response.msg[1] = 0x0B;           // Commande retour vitesse max
        break;
      case 0x0C :     // Envoi vitesse    
          buf = f.msg[3] * 0x100 + f.msg[4];
          servos.setMaxSpeed( servoId, buf );
        break;
        
//      // Gestion couple max
//      case 0x0D:      // Demande couple max
//          response.msg[1] = 0x0E;           // Commande retour couple max
//          buf = servos.getMaxTorque( servoId );
//          response.msg[3] = buf / 0x100;    // [MSB] couple max
//          response.msg[4] = buf % 0x100;    // [LSB] couple max
//        break;
//      case 0x0F :     // Envoi couple max    
//          buf = f.msg[3] * 0x100 + f.msg[4];
//          servos.setMaxTorque( servoId, buf );
//        break;
//
//      // Gestion couple
//      case 0x10 :     // Demande couple
//          response.msg[1] = 0x11;           // Commande retour couple
//          buf = servos.getTorque( servoId );
//          response.msg[3] = buf / 0x100;    // [MSB] couple
//          response.msg[4] = buf % 0x100;    // [LSB] couple
//        break;
        
      // Gestion accel
      case 0x12:      // Demande accel
          response.msg[1] = 0x13;           // Commande retour accel
        break;
      case 0x14 :     // Envoi couple max    
          buf = f.msg[3] * 0x100 + f.msg[4];
          servos.setAccel( servoId, buf );
        break;

        // Gestion mouvement avec position cible
        case 0x15 :     // Envoi couple max    
          buf = f.msg[3] * 0x100 + f.msg[4];
          servos.setTargetPosition( servoId, buf );
        break;

        // Gestion trajectoire
        case 0x16 :     // Envoi trajectoire
            unsigned int target = f.msg[3] * 0x100 + f.msg[4];
            unsigned int vmax = f.msg[5] * 0x100 + f.msg[6];
            unsigned int accel = f.msg[7];
            servos.setTrajectory( servoId, target, vmax, accel );
        break;
        
      
    }

    // CheckSum
    //response.msg[7] = (response.id / 0x100) ^ (response.id % 0x100);
    //for( int i=0; i<7; i++ ) response.msg[7] ^= response.msg[i];
    
    // Retour réponse
    CAN.sendMsgBuf( response.id, 0, response.size, response.msg );               // Envoi du message sur le bus CAN
    
  }

}


