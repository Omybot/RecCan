#include <Ethernet.h>
#include <EEPROM.h>
#include "mcp_can.h"

//#define DEBUG_EN  1

// Gestion ETHERNET

#define ETH_CS_PIN  9
#define ETH_PACKETSIZE 13

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};                // Adresse mac de la Maison du Can
IPAddress localIp(192, 168, 1, 15);                               // Adresse ip de la Maison du Can
IPAddress remoteIp(192, 168, 1, 26);                              // Adresse ip de la Maison du Can
unsigned int localPort = 12315;                                   // port sur lequel écouter
unsigned int remotePort = 64144;

byte udpId = 0xC5;                                                // Identifiant Udp de la Maison du CAN
byte udpEnvoiCan = 0xC0;                                          // Commande Udp utilisée pour les envoi CAN
byte udpReponseCan = 0xC1;                                        // Commande Udp utilisée pour les retours CAN

EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

// Gestion CAN

#define CAN_CS_PIN  10
#define CAN_FRAMESIZE 8

MCP_CAN CAN(CAN_CS_PIN);

unsigned char flagRecv = 0;

void MCP2515_ISR(){
  flagRecv = 1;
}

// Fonction qui initialise masques et filtres pour le bus CAN
void initCANMasksAndFilters() {

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

  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, localIp);                                    // Initialisation connection Ethernet
  Udp.begin(localPort);

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

      // Envoi sur le réseau Ethernet
      uint8_t udpPacketBuffer[ETH_PACKETSIZE];
      udpPacketBuffer[0] = udpId;
      udpPacketBuffer[1] = udpReponseCan;
      udpPacketBuffer[2] = ETH_PACKETSIZE - 3;
      udpPacketBuffer[3] = canId / 0x100;
      udpPacketBuffer[4] = canId % 0x100;
      for( int i=0 ; i<CAN_FRAMESIZE ; i++ ){
        if( i < canMsgSize ) udpPacketBuffer[i+5] = canMsg[i];
        else udpPacketBuffer[i] = 0;
      }
      Udp.beginPacket(remoteIp, remotePort);
      Udp.write(udpPacketBuffer, ETH_PACKETSIZE);
      Udp.endPacket();

    }

  }

  // Si reception packet ETHERNET
  int udpPacketSize = Udp.parsePacket();                          // Test si paquet Ethernet recu
  if( udpPacketSize ){

    // Récupération du packet ETHERNET
    uint8_t udpPacketBuffer[udpPacketSize];
    Udp.read(udpPacketBuffer, udpPacketSize);

    // Affichage packet recu
    
    #ifdef DEBUG_EN
    Serial.print( "\nNouvelle reception ETHERNET (taille : " );
    if( udpPacketSize < 10 ) Serial.print( " " );
    Serial.print( udpPacketSize );
    Serial.print(") => ");
    for( int i=0 ; i<udpPacketSize ; i++ ){
      if( udpPacketBuffer[i] < 0x10 ) Serial.print( "0" );
      Serial.print( udpPacketBuffer[i], HEX );
      if( i < udpPacketSize-1 ) Serial.print( "-" );
    }
    Serial.println();
    #endif

    // Test si trame à envoyer
    if( udpPacketSize == ETH_PACKETSIZE
         && udpPacketBuffer[0] == udpId
         && udpPacketBuffer[1] == udpEnvoiCan
         && udpPacketBuffer[2] == ETH_PACKETSIZE - 3 ){

      unsigned int canId = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
      uint8_t canMsg[CAN_FRAMESIZE];
      for( uint8_t i=0 ; i<CAN_FRAMESIZE ; i++ ) canMsg[i] = udpPacketBuffer[i + 5];

      // Affichage trame CAN à envoyer
      
      #ifdef DEBUG_EN
      Serial.print( "Nouvel envoi CAN => " );
      printCANFrame( canId, canMsg, CAN_FRAMESIZE );
      #endif

      // Envoi trame CAN
      bool sendStatus = CAN.sendMsgBuf( canId, 0, CAN_FRAMESIZE, canMsg, 0);
      if( sendStatus != CAN_OK ){
        #ifdef DEBUG_EN
        Serial.println( "Envoi KO !!!!!" );
        #endif
        udpPacketBuffer[0] = 0xFF;
        udpPacketSize = 1;
        Udp.beginPacket(remoteIp, remotePort);
        Udp.write(udpPacketBuffer, udpPacketSize);
        Udp.endPacket();
      }

    } else {

      #ifdef DEBUG_EN
      Serial.println( "Retour packet à l'envoyeur" );
      #endif

      // Renvoi packet ethernet à l'envoyeur
      Udp.beginPacket(remoteIp, remotePort);
      Udp.write(udpPacketBuffer, udpPacketSize);
      Udp.endPacket();

    }

  }

}
