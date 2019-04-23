#include <EEPROM.h>
#include <mcp_can.h>

//#define DEBUG_EN  1

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

  while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);
  initCANMasksAndFilters();
  attachInterrupt(0, MCP2515_ISR, FALLING);

  #ifdef DEBUG_EN
  Serial.println( "Init OK" );
  #endif
  
}

void loop(){
  
  // Si reception trame CAN
  if( flagRecv ){
    flagRecv = 0;
    
    while( CAN_MSGAVAIL == CAN.checkReceive() ){

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

      // Envoi réponse
      canId = 0x0001;
      bool sendStatus = CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0);
      if( sendStatus != CAN_OK ){
        #ifdef DEBUG_EN
        Serial.println( "Envoi KO !!!!!" );
        #endif
      }
      
      // Affichage trame envoyée
      #ifdef DEBUG_EN
      Serial.print( "\nNouvelle envoi CAN => " );
      printCANFrame( canId, canMsg, canMsgSize );
      #endif
      
    }
    
  }
  
}
