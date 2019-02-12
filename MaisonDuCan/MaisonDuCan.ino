#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>                                         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "Can.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};                // Adresse mac de la Maison du Can
IPAddress localIp(192, 168, 1, 15);                               // Adresse ip de la Maison du Can
IPAddress remoteIp(192, 168, 1, 27);
unsigned int localPort = 12315;                                   // port sur lequel écouter
unsigned int remotePort = 12325;                                  // port avec lequel envoyer

byte udpId = 0xC5;                                                // Identifiant Udp
byte udpEnvoiCan = 0xC0;                                          // Commande Udp utilisée pour les envoi CAN
byte udpReponseCan = 0xC1;                                        // Commande Udp utilisée pour les retours CAN

unsigned int retryCanSendTimeout = 50;                            // Temps max que doit prendre un envoi sur bus CAN

EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

void handleCanPackets();                                          // Fonction qui récupère les packets CAN et les envoient sur Ethernet en UDP tant qu'il y en a
void sendBufferToEth( uint8_t *udpBuffer, int bufSize );          // Fonction qui permet d'envoyer un buffer de taille définie sur le réseau Ethernet en UDP
void sendCanPacketToEth( canPacket p );                           // Fonction qui permet d'envoyer un paquet CAN ( 2 octets pour id + 8 octets pour message ) sur le réseau Ethernet en UDP en ajoutant l'entête des cartes RecCan

//////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////

void setup() {
  
  CAN.begin(CAN_80KBPS, MCP_8MHz);                                // Initialisation controleur CAN

  Ethernet.begin(mac, localIp);                                   // Initialisation connection Ethernet
  Udp.begin(localPort);

}

//////////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////////

void loop() {

  // CAN -> ETH
  handleCanPackets();

  // ETH -> CAN
  int udpPacketSize = Udp.parsePacket();                          // Test si paquet Ethernet recu
  if( udpPacketSize ){                                            // et pas vide

    // Récupération packet ethernet udp et renvoi automatique pour valider récéption
    uint8_t udpPacketBuffer[udpPacketSize];
    Udp.read(udpPacketBuffer, udpPacketSize);

    // Test si trame correspond à une demande d'envoi valide sur le bus CAN
    if( udpPacketSize == 13 && udpPacketBuffer[0] == udpId && udpPacketBuffer[1] == udpEnvoiCan && udpPacketBuffer[2] == 0x0A ){

      // Construction du packet CAN à partir du packet Ethernet recu
      canPacket p;
      p.id = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
      for( uint8_t i=0; i<8; i++ ) p.msg[i] = udpPacketBuffer[i+5];

      // Tentative d'envoi jusqu'à réussite ou timeout dépassé
      unsigned long startTime = millis();
      bool success = false;
      do{
        handleCanPackets();                                       // Lecture des paquets recu avant chaque tentative d'envoi pour être sur de ne pas en rater
        if( CAN.sendPacket(p) == CAN_OK ) success = true;
      } while( (millis() - startTime) < retryCanSendTimeout && !success );

    } else {
      
      sendBufferToEth( udpPacketBuffer, udpPacketSize );          // Renvoi de la trame test de connection
        
    }
    
  }
  
}

//////////////////////////////////////////////////////
// Fonctions
//////////////////////////////////////////////////////

// Fonction qui récupère les packets CAN et les envoient sur Ethernet en UDP tant qu'il y en a
void handleCanPackets(){
  while( CAN.checkNewPacket() ){                                  // Test si paquets CAN recu   
    canPacket p = CAN.getNewPacket();                             // Récupération d'un packet CAN
    sendCanPacketToEth( p );                                      // Envoi sur ethernet du packet CAN
  }
}

// Fonction qui permet d'envoyer un buffer de taille définie sur le réseau Ethernet en UDP
void sendBufferToEth( uint8_t *udpBuffer, int bufSize ){
  Udp.beginPacket(remoteIp, remotePort);
  Udp.write(udpBuffer, bufSize);
  Udp.endPacket();
}

// Fonction qui permet d'envoyer un paquet CAN ( id sur 2 octets + message de 8 octets ) sur le réseau Ethernet en UDP en ajoutant l'entête des cartes RecCan
void sendCanPacketToEth( canPacket p ){
  uint8_t buf[13] = { udpId, udpReponseCan, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  buf[3] = p.id / 0x100;                                          // Ajout identifiant CAN
  buf[4] = p.id % 0x100;
  for( uint8_t i=0; i<8; i++ ) buf[i+5] = p.msg[i];               // Ajout message CAN
  sendBufferToEth( buf, 13 );                                     // Envoi accusé de récéption (recopie du packet ethernet recu)
}
