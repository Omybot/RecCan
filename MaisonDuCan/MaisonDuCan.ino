#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>                                     // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "Can.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress localIp(192, 168, 1, 173);
unsigned int localPort = 12315;                               // local port to listen on
IPAddress remoteIp(192, 168, 1, 27);
unsigned int remotePort = 58488;                              // local port to listen on

EthernetUDP Udp;                                              // An EthernetUDP instance to let us send and receive packets over UDP

uint8_t packetBuffer[13];

void setup() {

  Serial.begin(500000);
  
  CAN.begin(CAN_80KBPS, MCP_8MHz);                                // Initialisation controleur CAN

  Ethernet.begin(mac, localIp);
  Udp.begin(localPort);

}

unsigned long tic, toc;

void loop() {

  tic = micros();
  
  while( CAN.checkNewPacket() ){                                  // Tant que packet CAN à lire
    
    packet p = CAN.getNewPacket();                                // Récupération du packet CAN
    
    packetBuffer[0] == 0xC5;                                      // Entête packet ETH
    packetBuffer[1] == 0xA5;
    packetBuffer[2] == 0x0A;
    packetBuffer[3] = p.id / 0x100;                               // Ajout du packet CAN au buffer
    packetBuffer[4] = p.id % 0x100;
    for( uint8_t i=0; i<8; i++ ) packetBuffer[i+5] = p.msg[i];
    
    Udp.beginPacket(remoteIp, remotePort);                        // Envoi du packet CAN sur Ethernet
    Udp.write(packetBuffer, 13);
    Udp.endPacket();
    
    toc = micros();
    Serial.print( "Reception Can + envoi Eth : " );
    Serial.print( toc - tic );
    Serial.println( " µs" );
    
  }

  int packetSize = Udp.parsePacket();
  if(packetSize){
    
    Udp.read(packetBuffer, packetSize);                           // Lecture du packet ethernet
    
    if( packetSize == 13 && packetBuffer[0] == 0xC5 && packetBuffer[1] == 0xA5 && packetBuffer[2] == 0x0A ){
      packet p;
      p.id = packetBuffer[3] * 0x100 + packetBuffer[4];
      for( uint8_t i=0; i<8; i++ ) p.msg[i] = packetBuffer[i+5];
      bool success = CAN.sendPacket( p );                         // Envoi packet sur bus CAN
      //if( !success ){
      //  ??? Bus peut etre pas libre ... / Trame toujours en cours d'envoi ... / ???
      //}
    }
    
    Udp.beginPacket(remoteIp, remotePort);                        // Renvoi automatique du packet recu
    Udp.write(packetBuffer, packetSize);
    Udp.endPacket();
    
    toc = micros();
    Serial.print( "Check CAN false + Reception Eth + Envoi CAN + Retour Eth : " );
    Serial.print( toc - tic );
    Serial.println( " µs" );
    
  }

}
