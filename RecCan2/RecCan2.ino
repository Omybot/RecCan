#include <EEPROM.h>
#include <mcp_can.h>

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

// Fonction qui initialise masques et filtres pour le bus CAN
void initCANMasksAndFilters(){

  // Récupération boardId  
  uint16_t boardId;
  EEPROM.get(0, boardId);                 // Récupération id de la carte
  
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

unsigned long time;
unsigned long stepTime = 500;

void setup(){
  
  Serial.begin(500000);

  while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);
  initCANMasksAndFilters();
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

  if( millis() > time + stepTime ){
    time += stepTime;
    
    buf[7] = buf[7]+1;  
    bool sendStatus = CAN.sendMsgBuf( 0x0001, 0, CAN_FRAMESIZE, buf, 0);
    if( sendStatus == CAN_OK ){
      Serial.println( "Envoi OK" );
    } else {
      Serial.println( "Envoi KO !!!!!" );
    }
    
  }
  
}
