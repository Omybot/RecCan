#include <mcp_can.h>

#define SPI_CS_PIN 8
MCP_CAN CAN(SPI_CS_PIN);

// Déclaration structure pour une trame
struct canFrame{
  unsigned int id;
  unsigned char size;
  unsigned char msg[8];
};

canFrame f;

byte txBuf[10];
byte rxBuf[10];
int rxBufIndex = 0;

void setup() {
  
  Serial.begin(19200);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) delay(10);     // Initialisation bus CAN

}

void loop() {

  // CAN -> UART
  if( CAN_MSGAVAIL == CAN.checkReceive() ){                             // Test si des trames ont été recues
    
    CAN.readMsgBuf( &f.size, f.msg );
    f.id = CAN.getCanId();
    
    txBuf[0] = f.id / 0x100;
    txBuf[1] = f.id % 0x100;
    
    for( int i = 0; i<f.size; i++ )
      txBuf[i+2] = f.msg[i];
      
    for( int j = f.size; j<10; j++ )
      txBuf[j+2] = f.msg[j];
      
    for( int k=0; k<10; k++ )                     // Envoi sur port de 10 octets (2 pour l'id + 8 pour le message)
      Serial.write( txBuf[k] );

  }

  // UART -> CAN
  if( Serial.available() ){
    
    rxBuf[ rxBufIndex++ ] = Serial.read();         // Récéption des octets 1 par 1
    
    if( rxBufIndex == 10 ){                        // 10 octets correspond à une trame complete
      rxBufIndex = 0;

      // Création trame CAN
      canFrame f;
      f.id = rxBuf[0] * 0x100 + rxBuf[1];

      if( f.id != 0 ){
        f.size = 8;
        for( int i=0; i<f.size; i++ ){
          f.msg[i] = rxBuf[i+2];
        }
  
        // Envoi sur le bus
        CAN.sendMsgBuf( f.id, 0, f.size, f.msg );
      }
      
    }
  }

}
