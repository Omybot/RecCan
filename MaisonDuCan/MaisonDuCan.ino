#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>                                         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include "Can.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};                // Adresse mac de la Maison du Can
IPAddress localIp(192, 168, 1, 15);                               // Adresse ip de la Maison du Can
IPAddress remoteIp(192, 168, 1, 26);                              // Adresse ip de la Maison du Can
unsigned int localPort = 12315;                                   // port sur lequel écouter
unsigned int remotePort = 12325;                                  // port avec lequel envoyer

byte udpId = 0xC5;                                                // Identifiant Udp
byte udpEnvoiCan = 0xC0;                                          // Commande Udp utilisée pour les envoi CAN
byte udpReponseCan = 0xC1;                                        // Commande Udp utilisée pour les retours CAN

unsigned int retryCanSendTimeout = 100;                           // Temps max (ms) que doit prendre un envoi sur bus CAN

EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

void handleCanPackets();                                          // Fonction qui récupère les packets CAN et les envoient sur Ethernet en UDP tant qu'il y en a

void sendBufferToEth( uint8_t *udpBuffer, int bufSize );          // Fonction qui permet d'envoyer un buffer de taille définie sur le réseau Ethernet en UDP
void sendCanPacketToEth( canPacket p );                           // Fonction qui permet d'envoyer un paquet CAN ( 2 octets pour id + 8 octets pour message ) sur le réseau Ethernet en UDP en ajoutant l'entête des cartes RecCan

//////////////////////////////////////////////////////
// SETUP
//////////////////////////////////////////////////////

void setup() {  

  Serial.begin(500000);
  
  CAN.begin(CAN_125KBPS, MCP_8MHz);                               // Initialisation controleur CAN

  Ethernet.begin(mac, localIp);                                   // Initialisation connection Ethernet
  Udp.begin(localPort);

  Serial.println("ok");

}

//////////////////////////////////////////////////////
// LOOP
//////////////////////////////////////////////////////

void loop() {

  // CAN -> ETH
  handleCanPackets();                                             // 500 µs maxi depuis dernier check

  // ETH -> CAN
  int udpPacketSize = Udp.parsePacket();                          // Test si paquet Ethernet recu
  if( udpPacketSize ){                                            // et pas vide

    handleCanPackets();                                           // 356 µs maxi depuis dernier check

    // Récupération packet ethernet udp et renvoi automatique pour valider récéption
    uint8_t udpPacketBuffer[udpPacketSize];
    Udp.read(udpPacketBuffer, udpPacketSize);
    
    // Test si trame correspond à une demande d'envoi valide sur le bus CAN
    if( udpPacketSize == 13 && udpPacketBuffer[0] == udpId && udpPacketBuffer[1] == udpEnvoiCan && udpPacketBuffer[2] == 0x0A ){

      canPacket p;                                                // Construction du packet CAN à partir du packet Ethernet recu
      p.id = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
      for( uint8_t i=0; i<8; i++ ) p.msg[i] = udpPacketBuffer[i+5];

      // Tentative d'envoi jusqu'à réussite ou timeout dépassé
      unsigned long startTime = millis();
      bool success = false;
      do {
        handleCanPackets();                                       // 248 µs maxi depuis dernier check
        if( CAN.sendPacket(p) == CAN_OK ) success = true;         // Tentative d'envoi sur bus CAN
      } while( (millis() - startTime) < retryCanSendTimeout && !success );

    } else {
      
      handleCanPackets();                                         // 248 µs maxi depuis dernier check 
      sendBufferToEth( udpPacketBuffer, udpPacketSize );          // Renvoi de la trame test de connection

    }
    
  }
  
}

//////////////////////////////////////////////////////
// Fonctions
//////////////////////////////////////////////////////

// Fonction qui récupère les packets CAN et les envoient sur Ethernet en UDP tant qu'il y en a (520 µs par packet)
// Le composant MCP2515 peut utiliser un buffer pour stocker une seconde trame -> Le temps d'execution 
// entre 2 appels de cette fonction ne doit pas dépasser 2x le temps d'envoi d'une trame de 108 octets
// 125kbps => 864µs par trame 
// Temps disponible entre 2 appels = ( 864 * 2 - 520 * 2 ) = 688µs maxi
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
  uint8_t sendBuf[13];
  sendBuf[0] = udpId;
  sendBuf[1] = udpReponseCan;
  sendBuf[2] = 0x0A;
  sendBuf[3] = p.id / 0x100;                                          // Ajout identifiant CAN
  sendBuf[4] = p.id % 0x100;
  for( uint8_t i=0; i<8; i++ ) sendBuf[i+5] = p.msg[i];               // Ajout message CAN
  sendBufferToEth( sendBuf, 13 );                                     // Envoi accusé de récéption (recopie du packet ethernet recu)
}
