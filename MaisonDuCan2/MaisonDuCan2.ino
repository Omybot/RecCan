#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};                // Adresse mac de la Maison du Can
IPAddress localIp(192, 168, 1, 15);                               // Adresse ip de la Maison du Can
IPAddress remoteIp(192, 168, 1, 26);                              // Adresse ip de la Maison du Can
unsigned int localPort = 12315;                                   // port sur lequel écouter
unsigned int remotePort = 56738;    

byte udpId = 0xC5;                                                // Identifiant Udp de la Maison du CAN
byte udpEnvoiCan = 0xC0;                                          // Commande Udp utilisée pour les envoi CAN
byte udpReponseCan = 0xC1;                                        // Commande Udp utilisée pour les retours CAN

#define ETH_PACKETSIZE 13
#define CAN_FRAMESIZE 8

#define ETH_CS_PIN  9
EthernetUDP Udp;                                                  // An EthernetUDP instance to let us send and receive packets over UDP

void setup(){

  Serial.begin(500000);

  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, localIp);                                    // Initialisation connection Ethernet
  Udp.begin(localPort);

}

void loop(){
    
  // Si reception packet ETHERNET
  int udpPacketSize = Udp.parsePacket();                          // Test si paquet Ethernet recu
  if( udpPacketSize ){     

    // Récupération du packet ETHERNET
    uint8_t udpPacketBuffer[udpPacketSize];
    Udp.read(udpPacketBuffer, udpPacketSize);

    // Affichage packet recu
    Serial.print( "Nouveau packet ETHERNET (taille : " );
    if( udpPacketSize < 10 ) Serial.print( " " );
    Serial.print( udpPacketSize );
    Serial.print(") => ");
    for( int i=0; i<udpPacketSize; i++ ){
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
      for( uint8_t i=0; i<8; i++ ) canMsg[i] = udpPacketBuffer[i+5];

      // Affichage trame CAN à envoyer
      Serial.print( "Nouvel envoi CAN => ID : ");
      if( canId < 1000 ) Serial.print( " " );
      if( canId < 100 ) Serial.print( " " );
      if( canId < 10 ) Serial.print( " " );
      Serial.print( canId );
      Serial.print( ", MSG : " );
      for( int i=0; i<CAN_FRAMESIZE; i++ ){
        if( canMsg[i] < 0x10 ) Serial.print( "0" );
        Serial.print( canMsg[i], HEX );
        if( i < CAN_FRAMESIZE-1 ) Serial.print( "-" );
      }
      Serial.println();

      // Envoi trame CAN
      
    } else {
      
      // Renvoi packet ethernet à l'envoyeur
      Udp.beginPacket(remoteIp, remotePort);
      Udp.write(udpPacketBuffer, udpPacketSize);
      Udp.endPacket();
      
    }
    
  }
  
}
