#include <SPI.h>
#include <mcp_can.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


const uint16_t boardId = 0x10;																		// Id de la carte

// Gestion CAN

#define CAN_CS_PIN  8
MCP_CAN CAN(CAN_CS_PIN);

// Fonction qui initialise masques et filtres pour le bus CAN
void initCANMasksAndFilters(){

	// Buffer de récéption RXB0 (1 masque et 2 filtres associés)
	CAN.init_Mask( 0, 0, 0xFFFF );																// Par défaut, le masque 1 bloque tout type d'identifiant, seul les filtres peuvent autoriser les identifiants
	CAN.init_Filt( 0, 0, boardId );																// Filtre qui n'autorique que l'id de la carte
	CAN.init_Filt( 1, 0, 0 );																		// Filtre qui n'autorique que l'id 0

	// Buffer de récéption RXB1 (1 masque et 4 filtres associés)
	CAN.init_Mask( 1, 0, 0xFFFF );																// Par défaut, le masque 1 bloque tout type d'identifiant, seul les filtres peuvent autoriser les identifiants
	CAN.init_Filt( 2, 0, 0 );																		// Filtre qui n'autorique que l'id 0
	CAN.init_Filt( 3, 0, 0 );																		// Filtre qui n'autorique que l'id 0
	CAN.init_Filt( 4, 0, 0 );																		// Filtre qui n'autorique que l'id 0
	CAN.init_Filt( 5, 0, 0 );																		// Filtre qui n'autorique que l'id 0

}

// Gestion affichage graphique

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Commandes
#define ScoreSet 			0xA0
#define ScoreAsk 			0xA1
#define ScoreResponse 	0xA2
#define TestConnection 	0x19

// Variables globales

volatile unsigned int _score = 148;

void setup(){

	display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

	display.setTextSize(7);
	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.clearDisplay();
	display.println(_score);
	display.display();

	while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);
	initCANMasksAndFilters();

}

void loop(){

	if( CAN_MSGAVAIL == CAN.checkReceive() ){

		// Récupération message
		unsigned char canMsgSize;
		unsigned char canMsg[8];
		CAN.readMsgBuf(&canMsgSize, canMsg);
		unsigned long canId = CAN.getCanId();

		// Gestion des commandes
		unsigned char command = canMsg[0];
		unsigned char servoId = canMsg[1];

		canMsgSize = 0;

		switch( command ){

			case ScoreSet : {  // 0x00
				_score = canMsg[2] * 0x100 + canMsg[3];
				display.clearDisplay();
				display.setCursor(0,0);
				display.println(_score);
				display.display();
				break;
			}

			case ScoreAsk : { // 0x01
				canMsg[0] = ScoreResponse;	// 0x02
				canMsg[1] = 0x00;
				canMsg[2] = _score >> 8;
				canMsg[3] = _score & 0xFF;
				canMsgSize = 4;
				break;
			}

			case TestConnection : {	// 0x19
				canMsg[0] = TestConnection;
				canMsgSize = 1;
				break;
			}

		}

		// Test si réponse à envoyer
		if( canMsgSize > 0 ){
			CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0 );								// Envoi réponse
		}

	}

}
