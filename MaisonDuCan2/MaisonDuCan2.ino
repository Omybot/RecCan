#include <Ethernet.h>
#include <EEPROM.h>
#include "mcp_can.h"

// Gestion ETHERNET

#define ETH_CS_PIN  9
#define ETH_PACKETSIZE 13

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};                // Adresse mac de la Maison du Can
IPAddress localIp(192, 168, 1, 15);                               // Adresse ip de la Maison du Can
IPAddress remoteIp(192, 168, 1, 26);                              // Adresse ip de la Maison du Can
unsigned int localPort = 12315;                                   // port sur lequel écouter
unsigned int remotePort = 56738;

byte udpId = 0xC5;                                                // Identifiant Udp de la Maison du CAN
byte udpEnvoiCan = 0xC0;                                          // Commande Udp utilisée pour les envoi CAN
byte udpReponseCan = 0xC1;                                        // Commande Udp utilisée pour les retours CAN

EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

// Gestion CAN

#define CAN_CS_PIN  10
#define CAN_FRAMESIZE 8

MCP_CAN CAN(CAN_CS_PIN);

unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];

void MCP2515_ISR(){
  flagRecv = 1;
}

// Fonction qui initialise masques et filtres pour le bus CAN
void initCANMasksAndFilters() {

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

void setup(){

  Serial.begin(500000);

  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, localIp);                                    // Initialisation connection Ethernet
  Udp.begin(localPort);

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
      for( int i=0 ; i<len ; i++ ){
        if( buf[i] < 0x10 ) Serial.print( "0" );
        Serial.print( buf[i], HEX );
        if( i < len-1 ) Serial.print( "-" );
      }
      Serial.println();

      // Envoi sur le réseau Ethernet
      uint8_t udpPacketBuffer[ETH_PACKETSIZE];
      udpPacketBuffer[0] = udpId;
      udpPacketBuffer[1] = udpReponseCan;
      udpPacketBuffer[2] = ETH_PACKETSIZE - 3;
      udpPacketBuffer[3] = canId / 0x100;
      udpPacketBuffer[4] = canId % 0x100;
      for( int i=0 ; i<CAN_FRAMESIZE ; i++ ){
        if( i < len ) udpPacketBuffer[i+5] = buf[i];
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
    Serial.print( "\nNouveau packet ETHERNET (taille : " );
    if( udpPacketSize < 10 ) Serial.print( " " );
    Serial.print( udpPacketSize );
    Serial.print(") => ");
    for( int i=0 ; i<udpPacketSize ; i++ ){
      if( udpPacketBuffer[i] < 0x10 ) Serial.print( "0" );
      Serial.print( udpPacketBuffer[i], HEX );
      if( i < udpPacketSize-1 ) Serial.print( "-" );
    }
    Serial.println();

    // Test si trame à envoyer
    if( udpPacketSize == ETH_PACKETSIZE
         && udpPacketBuffer[0] == udpId
         && udpPacketBuffer[1] == udpEnvoiCan
         && udpPacketBuffer[2] == ETH_PACKETSIZE - 3 ){

      unsigned int canId = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
      uint8_t canMsg[CAN_FRAMESIZE];
      for( uint8_t i=0 ; i<CAN_FRAMESIZE ; i++ ) canMsg[i] = udpPacketBuffer[i + 5];

      // Affichage trame CAN à envoyer
      Serial.print( "Nouvel envoi CAN => ID : 0x");
      if( canId < 1000 ) Serial.print( "0" );
      if( canId < 100 ) Serial.print( "0" );
      if( canId < 10 ) Serial.print( "0" );
      Serial.print( canId, HEX );
      Serial.print( ", MSG : " );
      for( int i=0 ; i<CAN_FRAMESIZE ; i++ ){
        if( canMsg[i] < 0x10 ) Serial.print( "0" );
        Serial.print( canMsg[i], HEX );
        if( i < CAN_FRAMESIZE-1 ) Serial.print( "-" );
      }
      Serial.println();

      // Envoi trame CAN
      bool sendStatus = CAN.sendMsgBuf( canId, 0, CAN_FRAMESIZE, canMsg, 0);
      if( sendStatus == CAN_OK ){
        Serial.println( "Envoi OK" );
      } else {
        Serial.println( "Envoi KO !!!!!" );
      }

    } else {

      Serial.println( "Retour packet à l'envoyeur" );

      // Renvoi packet ethernet à l'envoyeur
      Udp.beginPacket(remoteIp, remotePort);
      Udp.write(udpPacketBuffer, udpPacketSize);
      Udp.endPacket();

    }

  }

}
