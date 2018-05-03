#include <EEPROM.h>
#include <MyQueue.h>
#include <SPI.h>
#include "mcp_can.h"
#include <MyServoHandler.h>

/////////////////////////////////
// Gestion bus CAN
/////////////////////////////////

  #define SPI_CS_PIN 8
  MCP_CAN CAN(SPI_CS_PIN);
  
  // Déclaration structure pour une trame
  struct canFrame{
    unsigned int id;
    unsigned char size;
    unsigned char msg[8];
  };
  
  // Stockage des trames
  volatile MyQueue<canFrame> framesFromCAN( 5 );              // Pile pour stocker les messages provenant du bus CAN
  volatile MyQueue<canFrame> framesToCAN( 5 );                // Pile pour stocker les messages à envoyer sur le bus CAN
  
  // Fonction déclenchée lors reception des trames
  void MCP2515_ISR(){
    while (CAN_MSGAVAIL == CAN.checkReceive()){
      canFrame f;
      CAN.readMsgBuf( &f.size, f.msg );
      f.id = CAN.getCanId();
      framesFromCAN.push( f );                                // Stockage du message provenant du bus CAN
    }
  }

  // Envoi de la dernière trame de la queue
  void handleFramesToCAN(){
    if( framesToCAN.count() > 0 ){
      canFrame f = framesToCAN.pop();
      CAN.sendMsgBuf( f.id, 0, f.size, f.msg );               // Envoi du message sur le bus CAN
    }
  }

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
  handleFramesToCAN();
  servos.timer2Interrupt(); 
}

/////////////////////////////////
// SETUP
/////////////////////////////////
void setup() {

  Serial.begin(115200);
  
  // Gestion des pins
  servos.attach();
  
  // Initialisation CAN
  while (CAN_OK != CAN.begin(CAN_500KBPS)){ delay(10); }

  // Fonction d'interruption quand récéption trame bus CAN
  attachInterrupt(0, MCP2515_ISR, FALLING);

  // Filtre pour ne prendre en compte que les trames désirées
  CAN.init_Mask(0, 0, 0x7ff); 
  CAN.init_Mask(1, 0, 0x7ff); 
  int canId = 1;
  CAN.init_Filt(0, 0, canId);

}

/////////////////////////////////
// LOOP
/////////////////////////////////
void loop() {
  
  if( !framesFromCAN.isEmpty() ){                             // Test si des trames ont été recues

    volatile canFrame f = framesFromCAN.pop();                // Dépilage de la derniere trame
    
    byte frameNumber = f.msg[0];
    byte command = f.msg[1];
    byte servoId = f.msg[2];

    Serial.println( "Nouvelle commande " );
    Serial.print( "id : " );
    Serial.println( f.id , HEX );
    Serial.print( "fum : " );
    Serial.println( frameNumber, HEX );
    Serial.print( "cmd : " );
    Serial.println( command , HEX );
    Serial.print( "servoid : " );
    Serial.println( servoId , HEX );
    
    canFrame response;
    response.id = f.id;
    response.msg[0] = frameNumber;
    for( int i=1; i<7; i++ ) response.msg[i] = 0x00;
    response.size = 8;

    unsigned int buf;
    
    switch( command ){
      // Demande position cible
      case 1:
          buf = servos.getPosition( servoId );
          response.msg[1] = 0x02;
          response.msg[2] = servoId;
          response.msg[3] = buf / 0x100;    // [MSB] position
          response.msg[4] = buf % 0x100;    // [LSB] position
        break;
      // Envoi position cible
      case 3 :      
          buf = f.msg[3] * 0x100 + f.msg[4];
          servos.setPosition( servoId, buf );
          response.msg[1] = 0x02;
          response.msg[2] = servoId;
          response.msg[3] = buf / 0x100;    // [MSB] position
          response.msg[4] = buf % 0x100;    // [LSB] position
        break;
    }

    response.msg[7] = (response.id / 0x100) ^ (response.id % 0x100);
    for( int i=0; i<7; i++ ) response.msg[7] ^= response.msg[i];
    
    // Retour réponse
    framesToCAN.push( response );
    
  }

}


