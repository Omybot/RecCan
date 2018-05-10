#ifndef MY_SERVO_HANDLER_H
#define MY_SERVO_HANDLER_H

#include <Arduino.h>
#include "MyServo.h"

class MyServoHandler {
	
	public :
	
		MyServoHandler();
		~MyServoHandler();
		
		// Inititalisation
		void attach();
		
		// Gestion des interruptions
		void timer1Interrupt();
		void timer2Interrupt();
		
		// Gestion position
		void setPosition( byte servoNumber, float position );
		float getPosition( byte servoNumber );
		
		// Gestion de la vitesse
		void setSpeed( byte servoNumber, float speed );
		float getSpeed( byte servoNumber );
		
		// Gestion de l'acceleration
		void setAccel( byte servoNumber, float accel );
		float getAccel( byte servoNumber );
		
		
	private :
	
		byte _currentServo;
		MyServo _servos[4];
	
		void initTimer1();
		void rearmTimer1( float code );
		void initTimer2();
		
		void handleNextServo();
		
};

#endif