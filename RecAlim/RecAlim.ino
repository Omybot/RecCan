//////////////////////////////////
// Librairies
//////////////////////////////////

#include <Adafruit_SSD1306.h>			// Librairie écran OLED I2C
#include "mcp_can.h"						// Librairie CAN
#include "pitches.h"						// Librairie pour gestion de la musique

//////////////////////////////////
// Déclaration des pins
//////////////////////////////////

const int greenLedPin	= 0;
const int redLedPin 		= 1;
const int speakerPin 	= 3;
const int vBatEnPin 		= 5;

const int vBatMesPin 	= A0;
const int vSwitchPin		= A1;
const int iSensPin 		= A2;

//////////////////////////////////
// Variables globales
//////////////////////////////////

// Variables et constantes écran
#define SCREEN_WIDTH 	128
#define SCREEN_HEIGHT	64
#define OLED_RESET     	-1 			// Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 100000UL, 100000UL);

// Variables et constantes CAN
#define CAN_CS_PIN  		10				// Chip select du composant MCP2515
MCP_CAN CAN( CAN_CS_PIN );

const uint16_t boardId = 10;																		// Id de la carte

// Variables compteurs de temps
unsigned long time;
unsigned long stepTime = 50;
unsigned long time2;
unsigned long stepTime2 = 500;

unsigned long pushTime = 0;

// Variables pour mesure tension et courant
const float VBATSEUIL		= 23;
const float VBATSEUILRELOU = 22.5;
#define VBUFSIZE		8
float vBatBuf[VBUFSIZE];
int vBatBufIndex = 0;
float iSensBuf[VBUFSIZE];
int iSensBufIndex = 0;
int vBatSeuilCpt = 0;

//////////////////////////////////
// Fonctions programme
//////////////////////////////////

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

// Initialisation de l'écran OLED
void initOLED(){

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	display.display();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.clearDisplay();

}

void powerOn(){	digitalWrite( vBatEnPin , HIGH ); }	// Allumage carte
void powerOff(){	digitalWrite( vBatEnPin , LOW ); }	// Exctinction carte

// Melodie allumage carte alim
void startMelody(){

	/*
	// Hockey melody
	int nbNotes = 8;
	int melody[nbNotes] = { 			NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_E4, NOTE_G4 	};
	int noteDurations[nbNotes] = { 	8, 		8, 		8, 		4, 		8, 		2 			};	// note durations: 4 = quarter note, 8 = eighth note, etc.:
	*/

	// boooooring melody
	int nbNotes = 2;
	int melody[nbNotes] = { 			NOTE_C3, NOTE_G3 	};
	int noteDurations[nbNotes] = { 	4, 		4 			};	// note durations: 4 = quarter note, 8 = eighth note, etc.:


	// iterate over the notes of the melody:
   for (int i = 0; i < nbNotes; i++) {

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

// Récupération tension batterie moyennée
float getVBatAvg(){

	// Calcul tension batterie
	float vBat = 0;
	for( int i=0 ; i<VBUFSIZE ; i++ ) vBat += vBatBuf[i];
	vBat = vBat / VBUFSIZE;						// calcul signal moyen
	vBat = vBat / 1024.0 * 5.0;				// conversion en tension arduino
	vBat = vBat / 24.9 * 25.0;					// calibration
	vBat = vBat * (10+47)/10;					// conversion en tension batterie

	return vBat;

}

// Récupération consommation moyennée
float getISensAvg(){

	// Calcul courant
	float iSens = 0;
	for( int j=0 ; j<VBUFSIZE ; j++ ) iSens += iSensBuf[j];
	iSens = iSens / VBUFSIZE;					// calcul signal moyen
	iSens = iSens / 1024.0 * 5.0;				// conversion en tension arduino
	iSens = iSens / (50.0 * 0.005);			// conversion en courant consommé

	return iSens;

}

// Récupération état bouton
bool getSwitchState(){

	float vSwtich = analogRead( vSwitchPin ) / 1024.0 * 5.0;
	if( vSwtich > 1.0 )
		return true;
	else
		return false;

}

//////////////////////////////////
// SETUP
//////////////////////////////////

void setup(){

	// Initalisation de pins
	pinMode( redLedPin, 		OUTPUT );
	pinMode( greenLedPin, 	OUTPUT );
	pinMode( speakerPin, 	OUTPUT );
	pinMode( vBatEnPin, 		OUTPUT );

	pinMode( vBatMesPin, 	INPUT );
	pinMode( vSwitchPin, 	INPUT );
	pinMode( iSensPin, 		INPUT );

	// Allumage carte alim
	powerOn();
	startMelody();

	// Initialisation écran OLED
	initOLED();

	// Init CAN
	while( CAN_OK != CAN.begin(CAN_500KBPS) ) delay(100);									// Initialisation communication CAN
	initCANMasksAndFilters();

	// Récupération temps après setup
	time = millis();
	time2 = millis();

}

//////////////////////////////////
// LOOP
//////////////////////////////////

void loop(){


	////////////////
	// Gestion Exctinction
	////////////////

	while( getSwitchState() ){							// Si bouton appuyé
		digitalWrite( greenLedPin, HIGH );
		if( pushTime == 0 ){							// Premier appui
			pushTime = millis();						// Enregistrement temps premier appui
			tone( speakerPin, NOTE_G3 );
		} else {
			if( millis() > pushTime + 1000 )	noTone( speakerPin );
			else if( millis() > pushTime + 800 ){	// Appui long
				tone( speakerPin, NOTE_C3 );
				powerOff();								// Mise hors tension
			}
		}
	}

	// Bouton non appuyé
	//if( millis() > pushTime + 1000 )	noTone( speakerPin );
	digitalWrite( greenLedPin, LOW );
	pushTime = 0;									// Remise à zéro compteur

	////////////////
	// Fin Gestion Exctinction
	////////////////

	////////////////
	// Acquisition mesures
	////////////////

	if( millis() > time + stepTime ){
		time += stepTime;

		// Enregistrement signal ADC mesure tension
		vBatBuf[vBatBufIndex++] = analogRead(vBatMesPin);
		if( vBatBufIndex >= VBUFSIZE ) vBatBufIndex = 0;		// Rotation buffer

		// Enregistrement signal ADC mesure courant
		iSensBuf[iSensBufIndex++] = analogRead(iSensPin);
		if( iSensBufIndex >= VBUFSIZE ) iSensBufIndex = 0;		// Rotation buffer

	}

	////////////////
	// Fin Acquisition mesures
	////////////////

	////////////////
	// Gestion infos
	////////////////
	if( millis() > time2 + stepTime2 ){
		time2 += stepTime2;

		// Récupération des mesures tension/courant
		float vBat = getVBatAvg();
		float iSens = getISensAvg();

		// Vérification seuil tension batterie
		if( vBat < VBATSEUILRELOU ){

			tone( speakerPin, NOTE_C5 );

		} else if( vBat < VBATSEUIL ){

			digitalWrite( redLedPin, !digitalRead(redLedPin) );			// LED rouge qui clignote

			if( vBatSeuilCpt++ == 1 )	tone( speakerPin, NOTE_C3 );		// BIP !!!!!!!!!!!!!
			else noTone( speakerPin );

			if( vBatSeuilCpt > 10 ) vBatSeuilCpt = 0;							// Incrémentation compteurs

		} else {

			digitalWrite( redLedPin, LOW );										// Extinction led rouge !

			if( vBatSeuilCpt > 0 ){
				//noTone( speakerPin );
				vBatSeuilCpt = 0;														// Reset compteur de bip
			}

		}

		////////////////
		// Envoi CAN
		////////////////

		// Mise en forme en entier mV et mA pour envoyer sur bus CAN
		unsigned int vBatmV = vBat * 1000;
		unsigned int iSensmA = iSens * 1000;

		// Envoi trame CAN avec les infos tension et courant
		unsigned long canId = boardId;
		unsigned char canMsgSize = 5;
		unsigned char canMsg[5];
		canMsg[0] = 0xF5;																// Fonction pour Gobot
		canMsg[1] = vBatmV / 0x100;
		canMsg[2] = vBatmV % 0x100;
		canMsg[3] = iSensmA / 0x100;
		canMsg[4] = iSensmA % 0x100;
		CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0 );

		////////////////
		// Fin Envoi CAN
		////////////////

		////////////////
		// Mise à jour Affichage
		////////////////

		display.clearDisplay();
		display.setCursor(0,0);

		display.print( "vBat:" );
		display.print( vBat , 1 );
		display.println( "V" );

		display.print( "I:" );
		display.print( iSens,3);
		display.println( "A" );

		display.display();

		////////////////
		// Fin Affichage
		////////////////

	}

	////////////////
	// Fin Gestion infos
	////////////////

	////////////////
	// Reception trame CAN pour la musique !
	////////////////

	if( CAN_MSGAVAIL == CAN.checkReceive() ){

		// Récupération message
		unsigned char canMsgSize;
		unsigned char canMsg[8];
		CAN.readMsgBuf(  &canMsgSize, canMsg );
		unsigned long canId = CAN.getCanId();

		// Gestion des commandes
		byte command = canMsg[0];

		canMsgSize = 0;																				// Retour à zero pour initier message de retour

		switch( command ){

			case 0xF6 : {

				unsigned int frequency = canMsg[1] * 0x100 + canMsg[2];
				unsigned long duration = canMsg[3] * 0x100 + canMsg[4];
				canMsgSize = 5;
				tone( speakerPin, frequency, duration );

				break;

			}

		}

		// Test si réponse à envoyer
		if( canMsgSize > 0 ){
			CAN.sendMsgBuf( canId, 0, canMsgSize, canMsg, 0 );								// Envoi réponse
		}

	}

	////////////////
	// Fin Reception trame CAN pour la musique !
	////////////////


}
