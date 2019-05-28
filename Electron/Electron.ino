#include <Servo.h>

const int servoPin = 9;
const int omronPin = A3;
const int pullupPin = A2;
const int windPin = 4;
const int ledPin = 2;

Servo servo;

unsigned long time;
unsigned long stepTime = 100;

void setup(){

	// Mesure capteur omron
	pinMode( pullupPin, OUTPUT );																	// 5V pullup pour mesure capteur omron
	digitalWrite( pullupPin, HIGH );
	pinMode( omronPin, INPUT );																	// Entrée capteur omron

	// Initialisation des pins temoins
	pinMode( LED_BUILTIN, OUTPUT );																// LED de sortie
	digitalWrite( LED_BUILTIN, LOW );
	pinMode( ledPin, OUTPUT );																		// LED de sortie N°2 !
	digitalWrite( ledPin, LOW );
	pinMode( windPin, OUTPUT );																	// Commande ventilateur animation
	digitalWrite( windPin, LOW );

	// Position par défaut du servo
	servo.attach(9);
	servo.write(90);

	delay(100);

}

void loop(){

	if( millis() > time + stepTime ){
		time += stepTime;

		byte gobotIsHere = digitalRead( omronPin );
		if( gobotIsHere ){
			servo.write(90);
			digitalWrite( LED_BUILTIN, HIGH );
			digitalWrite( ledPin, HIGH );
			digitalWrite( windPin, HIGH );
		} else {
			servo.write(10);
			digitalWrite( LED_BUILTIN, LOW );
			digitalWrite( ledPin, LOW );
			digitalWrite( windPin, LOW );
		}

	}

}
