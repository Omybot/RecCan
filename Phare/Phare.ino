#include <Adafruit_NeoPixel.h>

#define PIXELPIN	6
#define NUMPIXELS	12
Adafruit_NeoPixel pixels(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);

const int switch1Pin = A1;
const int switch2Pin = A2;
const int motorPin = 8;

unsigned long time;
unsigned long stepTime = 150;

int startPixel = 0;

void setup(){

	// Configuration des pins
	pinMode( switch1Pin, INPUT );
	pinMode( switch2Pin, INPUT );
	pinMode( motorPin, OUTPUT );

  Serial.begin(500000);

	// Initialisation LEDs
	pixels.begin();

  delay(1000);

	// Detection démarrage
	for( int j=0; j<50; j++ ){
		if( digitalRead(switch1Pin) == LOW )
      j=0;
		delay(1);
	}

	// Allumage moteur
	digitalWrite( motorPin, HIGH );

	// Detection phare en butée
	for( int k=0; k<50; k++ ){
		while( digitalRead(switch2Pin) == LOW );
		delay(1);
	}

	// Extinction moteur
	digitalWrite( motorPin, LOW );

	// Démarrage séquence chenillard LED avec la LOOP
	// ...

}

void loop(){

	if( millis() > time + stepTime ){
		time += stepTime;

   Serial.print("toto");
   Serial.println(" pouet");

		for( int i=0; i<NUMPIXELS; i++ ){
			if( i == (startPixel % NUMPIXELS) || i == ((startPixel+1) % NUMPIXELS) || i == ((startPixel+2) % NUMPIXELS) ){
				pixels.setPixelColor( i, pixels.Color(255, 255, 255) );
			} else {
				pixels.setPixelColor( i, pixels.Color(0, 0, 0) );
			}
		}

		pixels.show();

		startPixel++;
		if( startPixel >= NUMPIXELS ) startPixel = 0;

	}

}
