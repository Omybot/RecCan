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

// Variables compteurs de temps
unsigned long time;
unsigned long stepTime = 100;
unsigned long time2;
unsigned long stepTime2 = 500;

unsigned long pushTime = 0;

// Variables pour mesure tension et courant
#define VBATSEUIL		21
#define VBUFSIZE		4
float vBatBuf[VBUFSIZE];
int vBatBufIndex = 0;
float iSensBuf[VBUFSIZE];
int iSensBufIndex = 0;

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

// Récupération tension batterie
/*float getVBat(){

	float vBat = analogRead( vBatMesPin ) / 1024.0 * 5.0;
	vBat = vBat * (10+47)/10;

	return vBat;

}*/

// Récupération consommation
/*float getISens(){

	float iSens = analogRead( iSensPin ) / 1024.0 * 5.0;
	iSens = iSens / (50.0 * 0.005);

	return iSens;

}*/

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

	// Récupération temps après setup
	time = millis();
	time2 = millis();

}

//////////////////////////////////
// LOOP
//////////////////////////////////

void loop(){

	// Gestion Exctinction
	if( getSwitchState() ){							// Si bouton appuyé
		digitalWrite( greenLedPin, HIGH );
		if( pushTime == 0 ){							// Premier appui
			pushTime = millis();						// Enregistrement temps premier appui
		} else {
			if( millis() > pushTime + 1000 ){	// Si appui long
				powerOff();								// Mise hors tension
			}
		}
	} else {												// Bouton non appuyé
		digitalWrite( greenLedPin, LOW );
		pushTime = 0;									// Remise à zéro compteur
	}

	// Gestion mesure des tensions
	if( millis() > time + stepTime ){
		time += stepTime;

		// Enregistrement signal ADC mesure tension
		vBatBuf[vBatBufIndex++] = analogRead(vBatMesPin);
		if( vBatBufIndex >= VBUFSIZE ) vBatBufIndex = 0;			// Rotation buffer

		// Enregistrement signal ADC mesure courant
		iSensBuf[iSensBufIndex++] = analogRead(iSensPin);
		if( iSensBufIndex >= VBUFSIZE ) iSensBufIndex = 0;		// Rotation buffer

	}

	// Gestion affichage des infos
	if( millis() > time2 + stepTime2 ){
		time2 += stepTime2;

		// Calcul tension batterie
		float vBat = 0;
		for( int i=0 ; i<VBUFSIZE ; i++ ) vBat += vBatBuf[i];
		vBat = vBat / VBUFSIZE;						// calcul signal moyen
		vBat = vBat / 1024.0 * 5.0;				// conversion en tension arduino
		vBat = vBat / 24.9 * 25.0;					// calibration
		vBat = vBat * (10+47)/10;					// conversion en tension batterie

		// Vérification seuil tension batterie
		if( vBat < VBATSEUIL ){
			digitalWrite( redLedPin, HIGH );
		} else {
			digitalWrite( redLedPin, LOW );
		}

		// Calcul courant
		float iSens = 0;
		for( int j=0 ; j<VBUFSIZE ; j++ ) iSens += iSensBuf[j];
		iSens = iSens / VBUFSIZE;					// calcul signal moyen
		iSens = iSens / 1024.0 * 5.0;				// conversion en tension arduino
		iSens = iSens / (50.0 * 0.005);			// conversion en courant consommé

		// Mise à jour affichage
		display.clearDisplay();
		display.setCursor(0,0);

		display.print( "vBat:" );
		display.print( vBat , 1 );
		display.println( "V" );

		display.print( "I:" );
		display.print( iSens,3);
		display.println( "A" );

		display.display();

	}

}
