#include <mcp_can.h>
#include <MyQueue.h>

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
  MyQueue<canFrame> framesToCAN( 10 );                    // Pile de trames a envoyer

  // Fonction d'interruption MCP2515 -> Nouveau message recu
  void MCP2515_ISR(){
    while( CAN_MSGAVAIL == CAN.checkReceive() ){          // Tant que des messages sont disponibles
      canFrame f;
      CAN.readMsgBuf( &f.size, f.msg );                   // Récupération taille du message et message
      f.id = CAN.getCanId();                              // Récupération id de la trame
      sendFrameToUART( f );                               // Envoi sur UART
    }
  }
  
/////////////////////////////////
// Fin Gestion bus CAN
/////////////////////////////////

/////////////////////////////////
// Gestion UART
/////////////////////////////////

  byte rxBuf[10];
  int rxBufIndex = 0;
  
  // Envoi d'une trame sur le port Série
  void sendFrameToUART( canFrame f ){
    
    byte txBuf[10] = {0,0,0,0,0,0,0,0,0,0};
  
    // Préparation des données
    txBuf[0] = f.id / 0x100;
    txBuf[1] = f.id % 0x100;
    for( int i = 0; i<f.size; i++ )
      txBuf[i+2] = f.msg[i];
  
    // Envoi sur port de 10 octets (2 pour l'id + 8 pour le message)
    for( int k=0; k<10; k++ )
      Serial.write( txBuf[k] );
    
  }

/////////////////////////////////
// Fin Gestion UART
/////////////////////////////////

/////////////////////////////////
// SETUP
/////////////////////////////////
void setup() {

  Serial.begin(19200);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) delay(10);     // Initialisation bus CAN
  attachInterrupt(0, MCP2515_ISR, FALLING);               // Fonction d'interruption quand récéption trame bus CAN

}

/////////////////////////////////
// LOOP
/////////////////////////////////
void loop() {

  // UART -> CAN
  if( Serial.available() ){
    
    rxBuf[ rxBufIndex++ ] = Serial.read();         // Récéption des octets 1 par 1
    
    if( rxBufIndex == 10 ){                               // 10 octets correspond à une trame complete
      rxBufIndex = 0;

      // Création trame CAN
      canFrame f;
      f.id = rxBuf[0] * 0x100 + rxBuf[1];
      f.size = 8;
      for( int i=0; i<f.size; i++ ){
        f.msg[i] = rxBuf[i+2];
      }

      // Envoi sur le bus
      CAN.sendMsgBuf( f.id, 0, f.size, f.msg );
      
    }
    
  }
  
}

