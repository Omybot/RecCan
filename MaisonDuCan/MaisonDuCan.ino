#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>                                         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "Can.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress localIp(192, 168, 1, 173);
unsigned int localPort = 12315;                                   // local port to listen on
IPAddress remoteIp(192, 168, 1, 27);
unsigned int remotePort = 53747;                                  // local port to listen on

EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

const int canRetryTime = 50;                                      // Temps max (en ms) d'une tentative de renvoi d'un paquet CAN sur le bus
bool canRetryFlag = false;                                        // Flag permettant d'indiquer qu'il faut refaire une tentative d'envoi
unsigned long canRetryTimeout;                                    // Variable qui permet de stocker le temps à partir duquel il faut abandonner la tentative de renvoi
canPacket canRetryPacket;                                         // Stockage tampon du packet CAN à renvoyer

// Fonction qui permet d'envoyer un buffer de taille définie sur le réseau Ethernet en UDP
void sendBufferToEth( uint8_t *udpBuffer, int bufSize ){
  Udp.beginPacket(remoteIp, remotePort);
  Udp.write(udpBuffer, bufSize);
  Udp.endPacket();
}

// Fonction qui permet d'envoyer un paquet CAN ( id sur 2 octets + message de 8 octets ) sur le réseau Ethernet en UDP en ajoutant l'entête des cartes RecCan
void sendCanPacketToEth( canPacket p ){
  uint8_t buf[13] = { 0xC5, 0xA5, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  buf[3] = p.id / 0x100;                                          // Ajout identifiant CAN
  buf[4] = p.id % 0x100;
  for( uint8_t i=0; i<8; i++ ) buf[i+5] = p.msg[i];               // Ajout message CAN
  sendBufferToEth( buf, 13 );                                     // Envoi accusé de récéption (recopie du packet ethernet recu)
}

void setup() {
  
  CAN.begin(CAN_80KBPS, MCP_8MHz);                                // Initialisation controleur CAN

  Ethernet.begin(mac, localIp);                                   // Initialisation connection Ethernet
  Udp.begin(localPort);

}

void loop() {

  // CAN -> ETH
  while( CAN.checkNewPacket() ){                                  // Test si paquets CAN recu   
    canPacket p = CAN.getNewPacket();                             // Récupération d'un packet CAN
    sendCanPacketToEth( p );                                      // Envoi sur ethernet du packet CAN
  }

  // ETH -> CAN
  if( canRetryFlag ){                                             // Si besoin de renvoyer un paquet
      if( millis() < canRetryTimeout ){                           // Si la tentative de renvoi est trop longue
        canRetryFlag = false;                                     // Abandon de la tentative de renvoi
      } else if(CAN.sendPacket(canRetryPacket) == CAN_OK){        // Sinon nouvelle tentative de renvoi
        sendCanPacketToEth(canRetryPacket);                       // Envoi sur ethernet du packet CAN si tentative = succees !
        canRetryFlag = false;                                     // Fin de la tentative de renvoi
      }
  } else {
    
    int udpPacketSize = Udp.parsePacket();                        // Test si paquet Ethernet recu
    if( udpPacketSize ){                                          // Si paquet n'est pas vide
      
      uint8_t udpPacketBuffer[13];
      Udp.read(udpPacketBuffer, udpPacketSize);                   // Récupération du packet ethernet recu
  
      if( udpPacketSize != 13 || udpPacketBuffer[0] != 0xC5 || udpPacketBuffer[1] != 0xA5 || udpPacketBuffer[2] != 0x0A ){ // Test si trame correspond à une demande d'envoi sur le bus CAN
        sendBufferToEth( udpPacketBuffer, udpPacketSize );        // Renvoi du paquet ethernet recu
      } else {
        canPacket p;                                              // Réconstruction du paquet CAN
        p.id = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
        for( uint8_t i=0; i<8; i++ ) p.msg[i] = udpPacketBuffer[i+5];
        
        if( CAN.sendPacket(p) == CAN_OK ){                        // Tentative d'envoi packet sur bus CAN
          sendCanPacketToEth(p);                                  // Envoi accusé de bonne transmission sur bus CAN
        } else {                                                  // Erreur envoi sur bus CAN
          canRetryFlag = true;                                    // Besoin d'effectuer une nouvelle tentative
          canRetryTimeout = millis() + canRetryTime;              // Sauvegarde du temps de la premiere demande de retry
          canRetryPacket = p;                                     // Sauvegarde du packet CAN à renvoyer
        }
      }
      
    }
    
  }

}
