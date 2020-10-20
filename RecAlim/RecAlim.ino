// Gestion écran
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 	128
#define SCREEN_HEIGHT	64
#define OLED_RESET     	-1 																		// Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 100000UL, 100000UL);

// Gestion can
#include "mcp_can.h"

#define CAN_CS_PIN  				10																	// Chip select du composant MCP2515
MCP_CAN CAN( CAN_CS_PIN );

// Gestion mélodie
#include "pitches.h"

//////////////////////////////////
// Déclaration des pins
//////////////////////////////////

const int vBatMesPin 	= A0;
const int vISensPin 		= A2;
const int ledPin 			= 1;
const int speakerPin 	= 3;
const int vBatEnPin 		= 5;

//////////////////////////////////
// Variables globales
//////////////////////////////////

int toto = 0;

unsigned long time;
unsigned long stepTime = 500;
unsigned long lastPushTime = 0;

//////////////////////////////////
// Fonctions programme
//////////////////////////////////

// Initialisation de l'écran OLED
void initOLED(){

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	display.display();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.clearDisplay();

}

// Melodie allumage carte alim
void startMelody(){

	int melody[] = {
	  NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_G4
	};

	// note durations: 4 = quarter note, 8 = eighth note, etc.:
	int noteDurations[] = { 8, 8, 8, 4, 8, 2 };

	// iterate over the notes of the melody:
   for (int i = 0; i < 8; i++) {

     // to calculate the note duration, take one second divided by the note type.
     //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
     int noteDuration = 1000 / noteDurations[i];
     tone(speakerPin, melody[i], noteDuration);

     // to distinguish the notes, set a minimum time between them.
     // the note's duration + 20% seems to work well:
     int pauseBetweenNotes = noteDuration * 1.20;
     delay(pauseBetweenNotes);
     // stop the tone playing:
     noTone(speakerPin);
   }

}

// Melodie extinction carte alim
void stopMelody(){

	int melody[] = {
	  NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_G4
	};

	// note durations: 4 = quarter note, 8 = eighth note, etc.:
	int noteDurations[] = { 8, 8, 8, 4, 8, 2 };

	// iterate over the notes of the melody:
   for (int i = 0; i < 8; i++) {

     // to calculate the note duration, take one second divided by the note type.
     //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
     int noteDuration = 1000 / noteDurations[i];
     tone(speakerPin, melody[i], noteDuration);

     // to distinguish the notes, set a minimum time between them.
     // the note's duration + 20% seems to work well:
     int pauseBetweenNotes = noteDuration * 1.20;
     delay(pauseBetweenNotes);
     // stop the tone playing:
     noTone(speakerPin);
   }

}

// Allumage carte
void powerOn(){

	digitalWrite( vBatEnPin , HIGH );

}

// Exctinction carte
void powerOff(){

	digitalWrite( vBatEnPin , LOW );

}

//////////////////////////////////
// SETUP
//////////////////////////////////

void setup(){

	// Initalisation de pins
	pinMode( vBatMesPin, INPUT );
	pinMode( vISensPin, 	INPUT );
	pinMode( ledPin, 		OUTPUT );
	pinMode( speakerPin, OUTPUT );
	pinMode( vBatEnPin, 	OUTPUT );

	// Allumage carte alim
	powerOn();
	startMelody();

	// Initialisation écran OLED
	initOLED();

	// Init CAN
	while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);									// Initialisation communication CAN

	// Récupération temps après setup
	time = millis();

}

//////////////////////////////////
// LOOP
//////////////////////////////////

void loop(){

	// Récupération tension batterie (quand bouton allumé !)
	float vBatMes = analogRead( vBatMesPin ) / 1024.0 * 5.0;
	float vBat = vBatMes * (11+47*2)/11;

	// Récupération courant alimentation
	float vISens = analogRead( vISensPin ) / 1024.0 * 5.0;
	float ISens = vISens / (50.0 * 0.005);

	// Test si bouton appuyé
	bool pushEnable = ( vBatMes > 1.0 ) ? true : false;									// 0 si pas appuyé, 2,51V pour vBat = 24V

	if( !pushEnable ){

		lastPushTime = millis();																	// Compteur à zéro
		noTone(speakerPin);																			// Pas de buzzer
		digitalWrite( ledPin, LOW );																// Pas de led

		// Mise à jour à intervalle régulier afficage OLED
		if( millis() > time + stepTime ){
			time += stepTime;

			// Envoi trame CAN
			unsigned long canId = 0x00;
			unsigned char canMsgSize = 4;
			unsigned char canMsg[4];
			canMsg[0] = 0x01;
			canMsg[1] = 0x02;
			canMsg[2] = 0x03;
			canMsg[3] = 0x04;
			if( CAN_OK == CAN.trySendMsgBuf( canId, 0, canMsgSize, canMsg, 0 ) ) toto++;

			// update display
			display.clearDisplay();
			display.setCursor(0,0);
			display.print("bouton : ");
			display.println( pushEnable );
			display.print("vBat:");
			display.print(vBat,1);
			display.println("V");
			display.print("I:");
			display.print(ISens,3);
			display.println("A");
			display.print(toto);
			display.print("  ");
			display.display();

		}

	} else {

		unsigned long int timePushed = millis() - lastPushTime;

		if( timePushed > 1000 ){

			powerOff();
			noTone( speakerPin );

		} else {


			if( (timePushed/100) % 2 == 0 ){

				// centaines de millis secondes paires
				digitalWrite( ledPin, HIGH );

			} else {

				// centaines de millis secondes impaires
				digitalWrite( ledPin, LOW );

			}

			/*if( timePushed >= 0 )
				tone( speakerPin, 500-timePushed/2 );*/
			if( timePushed > 800 )			tone( speakerPin, NOTE_C3 );
			else if( timePushed > 600 )	tone( speakerPin, NOTE_D3 );
			else if( timePushed > 400 )	tone( speakerPin, NOTE_E3 );
			else if( timePushed > 200 )	tone( speakerPin, NOTE_F3 );
			else									tone( speakerPin, NOTE_G3 );

		}

	}

}
