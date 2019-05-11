//#define DEBUG_EN  1

#include <Ethernet.h>
#include "mcp_can.h"

unsigned int cptCanIn, cptCanOut;

unsigned long time;
unsigned long stepTime = 1000;

// Gestion ETHERNET

#define ETH_CS_PIN  9

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};                // Adresse mac de la Maison du Can
//IPAddress localIp(192, 168, 1, 15);                               // Adresse ip de la Maison du Can
//IPAddress remoteIp(192, 168, 1, 26);                              // Adresse ip de Gobot
IPAddress localIp(10, 1, 0, 15);                               // Adresse ip de la Maison du Can
IPAddress remoteIp(10, 1, 0, 2);                              // Adresse ip de Gobot
unsigned int localPort = 12315;                                   // port sur lequel écouter
unsigned int remotePort = 12325;

byte udpId = 0xC5;                                                // Identifiant Udp de la Maison du CAN
byte udpEnvoiCan = 0xC0;                                          // Commande Udp utilisée pour les envoi CAN
byte udpReponseCan = 0xC1;                                        // Commande Udp utilisée pour les retours CAN

EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

#define ETH_MAX_PACKETSIZE 13
uint8_t udpPacketBuffer[ETH_MAX_PACKETSIZE];

// Gestion CAN

#define CAN_CS_PIN  10
#define CAN_MAX_FRAMESIZE 8

MCP_CAN CAN(CAN_CS_PIN);

unsigned int canId;
unsigned char canMsgSize;
uint8_t canMsg[CAN_MAX_FRAMESIZE];
unsigned char flagRecv = 0;

void MCP2515_ISR(){
  flagRecv = 1;
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

bool state = false;

void setup(){

  #ifdef DEBUG_EN
  Serial.begin(500000);
  #endif

  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, localIp);                                    // Initialisation connection Ethernet
  Udp.begin(localPort);

  while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);
  attachInterrupt(0, MCP2515_ISR, FALLING);

  #ifdef DEBUG_EN
  Serial.println( "Init OK" );
  #endif

  // Envoi trame bibon au début
  udpPacketBuffer[0] = udpId;
  udpPacketBuffer[1] = udpEnvoiCan;
  udpPacketBuffer[2] = 2;
  udpPacketBuffer[3] = 0xC5;
  udpPacketBuffer[4] = 0xF0;

  Udp.beginPacket(remoteIp, remotePort);
  Udp.write(udpPacketBuffer, 5);
  Udp.endPacket();
   delay(1000);

   // Envoi trame bibon au début
   udpPacketBuffer[0] = udpId;
   udpPacketBuffer[1] = udpEnvoiCan;
   udpPacketBuffer[2] = 2;
   udpPacketBuffer[3] = 0xC5;
   udpPacketBuffer[4] = 0xF0;

   Udp.beginPacket(remoteIp, remotePort);
   Udp.write(udpPacketBuffer, 5);
   Udp.endPacket();

}

void loop(){

  // Affichage compteur de trames
  if( millis() > time + stepTime ){
    time += stepTime;

    #ifdef DEBUG_EN
    Serial.print( "\ncptCan In/Out : " );
    Serial.print( cptCanIn );
    Serial.print( " / " );
    Serial.println( cptCanOut );
    #endif

  }

  // Si reception trame CAN
//  if( flagRecv ){
//    flagRecv = 0;

    if( CAN_MSGAVAIL == CAN.checkReceive() ){

      cptCanIn++;

      // Récupération message
      CAN.readMsgBuf(&canMsgSize, canMsg);
      canId = CAN.getCanId();

      // Affichage trame recue
      #ifdef DEBUG_EN
      Serial.print( "\nNouvelle reception CAN => " );
      printCANFrame( canId, canMsg, canMsgSize );
      #endif

      // Envoi sur le réseau Ethernet
      udpPacketBuffer[0] = udpId;
      udpPacketBuffer[1] = udpReponseCan;
      udpPacketBuffer[2] = 0x0A;//udpPacketBuffer[2] = canMsgSize+2;
      udpPacketBuffer[3] = canId / 0x100;
      udpPacketBuffer[4] = canId % 0x100;
      for( int i=0 ; i<canMsgSize ; i++ ) udpPacketBuffer[i+5] = canMsg[i];
      Udp.beginPacket(remoteIp, remotePort);
      Udp.write(udpPacketBuffer, 13);//Udp.write(udpPacketBuffer, canMsgSize+5);
      Udp.endPacket();

    }

  //}

  // Si reception packet ETHERNET
  int udpPacketSize = Udp.parsePacket();                          // Test si paquet Ethernet recu
  if( udpPacketSize ){

    remoteIp = Udp.remoteIP();

    // Récupération du packet ETHERNET
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
    if( udpPacketSize >= 5
          && udpPacketSize <= ETH_MAX_PACKETSIZE
          && udpPacketBuffer[0] == udpId
          && udpPacketBuffer[1] == udpEnvoiCan
          && udpPacketBuffer[2] > 2
          && udpPacketBuffer[2] <= CAN_MAX_FRAMESIZE+2 ){

      canId = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
      canMsgSize = udpPacketBuffer[2]-2;
      for( uint8_t i=0 ; i<canMsgSize ; i++ ) canMsg[i] = udpPacketBuffer[i + 5];

      // Affichage trame CAN à envoyer
      #ifdef DEBUG_EN
      Serial.print( "Nouvel envoi CAN => " );
      printCANFrame( canId, canMsg, canMsgSize );
      #endif

      // Envoi trame CAN
      bool sendStatus = CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0);
      if( sendStatus == CAN_OK ){
        cptCanOut++;
      } else {
        #ifdef DEBUG_EN
        Serial.println( "Envoi KO !!!!!" );
        #endif
        udpPacketBuffer[0] = 0xFF;
        udpPacketSize = 1;
        Udp.beginPacket(remoteIp, remotePort);
        Udp.write(udpPacketBuffer, 13);//Udp.write(udpPacketBuffer, udpPacketSize);
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
