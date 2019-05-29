#include <Ethernet.h>
#include "mcp_can.h"

/////////////////////////////////
// Gestion ETHERNET
/////////////////////////////////

#define ETH_CS_PIN  				9																	// Chip select du composant W5500
#define ETH_MAX_PACKETSIZE 	13																	// Taille maximale d'un paquet ethernet

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};											// Adresse mac de la Maison du Can
IPAddress localIp( 10, 1, 0, 15 );																// Adresse ip de la Maison du Can
IPAddress remoteIp( 10, 1, 0, 2 );																// Adresse ip de Gobot
unsigned int localPort = 12315;																	// port sur lequel écouter
unsigned int remotePort = 12325;

const byte udpId = 0xC5;																					// Identifiant Udp de la Maison du CAN
const byte udpEnvoiCan = 0xC0;																			// Commande Udp utilisée pour les envoi CAN
const byte udpReponseCan = 0xC1;																			// Commande Udp utilisée pour les retours CAN

EthernetUDP Udp;																						// An EthernetUDP instance to let us send and receive packets over UDP

/////////////////////////////////
// Gestion CAN
/////////////////////////////////

#define CAN_CS_PIN  				10																	// Chip select du composant MCP2515
#define CAN_MAX_FRAMESIZE 		8																	// Taille maximale d'une trame CAN

MCP_CAN CAN( CAN_CS_PIN );																			// Instance de MCP_CAN qui permet de gérer les communications CAN

/////////////////////////////////
// SETUP
/////////////////////////////////

void setup(){

	delay(200);

	// Init CAN
	while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);									// Initialisation communication CAN

	delay(200);

	// Init ETH
	Ethernet.init( ETH_CS_PIN );																	// Réglage chip select ethernet
	Ethernet.begin( mac, localIp );																// Initialisation connection Ethernet avec adresse MAC et IP
	while( Udp.begin( localPort ) != 1 ) delay(100);										// Initialisation communication udp sur le port désiré

	delay(200);

	// Envoi trame bibon au début
	byte udpPacketBuffer[2];
	udpPacketBuffer[0] = udpId;
	udpPacketBuffer[1] = 0xFF;
	Udp.beginPacket( remoteIp, remotePort );
	Udp.write( udpPacketBuffer, 2 );
	Udp.endPacket();

	delay(200);

}

/////////////////////////////////
// LOOP
/////////////////////////////////

void loop(){

	// Test reception nouvelle trame CAN
	if( CAN_MSGAVAIL == CAN.checkReceive() ){

		byte canMsgSize;
		byte canMsg[CAN_MAX_FRAMESIZE];

		// Récupération message
		CAN.readMsgBuf( &canMsgSize, canMsg );
		unsigned int canId = CAN.getCanId();

		// Envoi sur le réseau Ethernet
		byte udpPacketBuffer[ETH_MAX_PACKETSIZE];
		udpPacketBuffer[0] = udpId;
		udpPacketBuffer[1] = udpReponseCan;
		udpPacketBuffer[2] = 0x0A;
		udpPacketBuffer[3] = canId / 0x100;
		udpPacketBuffer[4] = canId % 0x100;
		for( int i=0 ; i<ETH_MAX_PACKETSIZE ; i++ ){
			if( i<canMsgSize ) udpPacketBuffer[i+5] = canMsg[i];
			else udpPacketBuffer[i+5] = 0x00;
		}

		Udp.beginPacket( remoteIp, remotePort );
		Udp.write( udpPacketBuffer, ETH_MAX_PACKETSIZE );
		Udp.endPacket();

	}

	// Test reception nouveau packet ETH
	int udpPacketSize = Udp.parsePacket();
	if( udpPacketSize ){

		remoteIp = Udp.remoteIP();																	// Mise à jour adresse IP envoyeur

		// Récupération du packet ETHERNET
		byte udpPacketBuffer[ETH_MAX_PACKETSIZE];
		Udp.read( udpPacketBuffer, udpPacketSize );

		// Test si trame à envoyer
		if( udpPacketSize >= 5
				&& udpPacketSize <= ETH_MAX_PACKETSIZE
				&& udpPacketBuffer[0] == udpId
				&& udpPacketBuffer[1] == udpEnvoiCan
				&& udpPacketBuffer[2] == 0x0A
		){

			unsigned int canId = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
			byte canMsgSize = 8;
			byte canMsg[CAN_MAX_FRAMESIZE];
			for( byte i=0 ; i<canMsgSize ; i++ ) canMsg[i] = udpPacketBuffer[i + 5];

			// Envoi trame CAN
			CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0 );

		} else {

			// Renvoi packet ethernet à l'envoyeur
			Udp.beginPacket( remoteIp, remotePort );
			Udp.write( udpPacketBuffer, udpPacketSize );
			Udp.endPacket();

		}

	}

}
