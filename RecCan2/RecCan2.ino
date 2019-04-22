#include <mcp_can.h>
#include <SPI.h>

// Gestion CAN

#define CAN_CS_PIN  8
#define CAN_FRAMESIZE 8

MCP_CAN CAN(CAN_CS_PIN); 

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void MCP2515_ISR(){
    flagRecv = 1;
}

void setup(){
  
  Serial.begin(500000);

  while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);
  attachInterrupt(0, MCP2515_ISR, FALLING);

  Serial.println( "Init OK" );
  
}

void loop(){
  
  // Si reception trame CAN
  if( flagRecv ){
    flagRecv = 0;
    
    while( CAN_MSGAVAIL == CAN.checkReceive() ){

      // Récupération message
      CAN.readMsgBuf(&len, buf);
      unsigned long canId = CAN.getCanId();

      // Affichage trame recue
      Serial.print( "\nNouvelle reception CAN => ID : 0x");
      if( canId < 1000 ) Serial.print( "0" );
      if( canId < 100 ) Serial.print( "0" );
      if( canId < 10 ) Serial.print( "0" );
      Serial.print( canId, HEX );
      Serial.print( ", MSG (" );
      Serial.print( len );
      Serial.print( ") : " );
      for( int i=0; i<len; i++ ){
        if( buf[i] < 0x10 ) Serial.print( "0" );
        Serial.print( buf[i], HEX );
        if( i < len-1 ) Serial.print( "-" );
      }
      Serial.println();
      
    }
    
  }
  
  buf[7] = buf[7]+1;  
  CAN.sendMsgBuf(0x42, 0, 8, buf);
  delay(500);
  
}
