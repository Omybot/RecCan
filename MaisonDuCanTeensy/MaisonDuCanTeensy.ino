#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <FlexCAN.h>

const int ledPin = 13;
const int resetPin = 6;
const int sckPin = 14;
const int csPin = 19;	// Chip select du composant W5500

/////////////////////////////////
// Gestion ETHERNET
/////////////////////////////////

#define ETH_MAX_PACKETSIZE 	13

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip( 10, 1, 0, 15 );															// Adresse ip de la Maison du Can
IPAddress remoteIp( 10, 1, 0, 5 );
unsigned int localPort = 12315;      																		// port sur lequel écouter
unsigned int remotePort = 12325;												// local port to listen on

const byte udpId = 0xC5;																					// Identifiant Udp de la Maison du CAN
const byte udpEnvoiCan = 0xC0;																			// Commande Udp utilisée pour les envoi CAN
const byte udpReponseCan = 0xC1;

EthernetUDP Udp;

unsigned long linkStatusTime = 0;
unsigned long linkStatusStepTime = 500;

/////////////////////////////////
// Gestion CAN
/////////////////////////////////																					// An EthernetUDP instance to let us send and receive packets over UDP

#define CAN_MAX_FRAMESIZE	8

static CAN_message_t msg;

void printCanMsg( CAN_message_t msg ){

	Serial.print( msg.id, HEX );
	Serial.print( " / " );
	for( int i = 0; i<msg.len; i++ ){
		Serial.print( "0x" );
		if( msg.buf[i] < 0x10 ) Serial.print( "0" );
		Serial.print( msg.buf[i], HEX );
		if( i<msg.len-1 ) Serial.print(  " - " );
	}
	Serial.println();

}

/////////////////////////////////
// SETUP
/////////////////////////////////

void setup(){

	pinMode(ledPin, OUTPUT);

	/////////////
	// Init ETH
	/////////////

	pinMode(resetPin, OUTPUT);
	digitalWrite(resetPin, LOW);
	delay(1);
	digitalWrite(resetPin, HIGH);

	Serial.begin(1000000);

	SPI.setSCK(sckPin);																						// SCK sur pin 14
	Ethernet.init(csPin);																					// CS sur pin 19
	Ethernet.begin(mac, ip);

	// Check for Ethernet hardware present
	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
		Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
		while (true) {
			digitalWrite( ledPin, LOW );
			delay(1); // do nothing, no point running without Ethernet hardware
		}
	}
	digitalWrite( ledPin, HIGH );
	Udp.begin(localPort);																				// start UDP

	/////////////
	// Init CAN
	/////////////

	Can0.begin();

}

void loop(){

	// Test status toutes les 500ms
	if( millis() > linkStatusTime + linkStatusStepTime ){
		linkStatusTime += linkStatusStepTime;
		digitalWrite( ledPin, !digitalRead(ledPin) );
		if( Ethernet.linkStatus() == LinkON ){
			//Serial.println("Link status: On");					// Lien Ethernet ok => clignottement toutes les secondes
			linkStatusStepTime = 500;
		} else {
		 	//Serial.println("Link status: Off or unknown");	// Lien Ethernet KO => clignottement toutes les quart de secondes
			linkStatusStepTime = 125;
		}

	}

	// if there's data available, read a packet
	int udpPacketSize = Udp.parsePacket();
	if( udpPacketSize ){

		remoteIp = Udp.remoteIP();          // Mise à jour adresse ip envoyeur
    
		// Récupération du packet ETHERNET
		byte udpPacketBuffer[ETH_MAX_PACKETSIZE];
		Udp.read( udpPacketBuffer, udpPacketSize );

		// Affichage des trames recues pour débug
		/*Serial.print( "New ETH from ip : " );
		for( int i=0; i<4; i++ ){
			Serial.print( remoteIp[i], DEC );
			if( i<3 ) Serial.print(".");
		}
		Serial.print( ", port : " );
		Serial.print( remotePort );
		Serial.print( ", size : " );
		Serial.println( udpPacketSize );*/

		// Test si trame à envoyer
		if( udpPacketSize >= 5
				&& udpPacketSize <= ETH_MAX_PACKETSIZE
				&& udpPacketBuffer[0] == udpId
				&& udpPacketBuffer[1] == udpEnvoiCan
				&& udpPacketBuffer[2] == 0x0A
		){

			// Préparation trame CAN
			msg.ext = 0;
			msg.id = udpPacketBuffer[3] * 0x100 + udpPacketBuffer[4];
			msg.len = CAN_MAX_FRAMESIZE;
			for( byte i=0 ; i<CAN_MAX_FRAMESIZE ; i++ ) msg.buf[i] = udpPacketBuffer[i + 5];

			// Affichage trame CAN à envoyer
			//Serial.print( ">>>> From Gobot => CanId : 0x" );
			//printCanMsg( msg );

			// Envoi trame CAN
			Can0.write(msg);

		} else {

			//Serial.println( ">>>> From Gobot => Packet inconnu :(" );

			// Renvoi packet ethernet à l'envoyeur
			Udp.beginPacket( remoteIp, remotePort );
			Udp.write( udpPacketBuffer, udpPacketSize );
			Udp.endPacket();

		}

	}

	// Récéption nouvelle trame CAN
	while( Can0.available() ){

    Serial.println("toto");

		// Récupération message
		Can0.read( msg );

		// Affichage message
		//Serial.print( "<<<< To Gobot => CanId : 0x" );
		//printCanMsg( msg );

		// Envoi sur le réseau Ethernet
		byte udpPacketBuffer[ETH_MAX_PACKETSIZE];
		udpPacketBuffer[0] = udpId;
		udpPacketBuffer[1] = udpReponseCan;
		udpPacketBuffer[2] = 0x0A;
		udpPacketBuffer[3] = msg.id / 0x100;
		udpPacketBuffer[4] = msg.id % 0x100;
		for( int i=0 ; i<ETH_MAX_PACKETSIZE ; i++ ){
			if( i<msg.len ) udpPacketBuffer[i+5] = msg.buf[i];
			else udpPacketBuffer[i+5] = 0x00;
		}

		Udp.beginPacket( remoteIp, remotePort );
		Udp.write( udpPacketBuffer, ETH_MAX_PACKETSIZE );
		Udp.endPacket();

	}

  delayMicroseconds(10);

}
