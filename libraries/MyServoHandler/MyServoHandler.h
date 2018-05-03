#ifndef MY_SERVO_HANDLER_H
#define MY_SERVO_HANDLER_H

#include <Arduino.h>
#include "MyServo.h"

class MyServoHandler {
	
	public :
	
		MyServoHandler();
		~MyServoHandler();
		
		void attach();
		
		void timer1Interrupt();
		void timer2Interrupt();
		
		// Paramètres
		void setMinPosition( byte servoNumber, unsigned int min );
		unsigned int getMinPosition( byte servoNumber );
		void setMaxPosition( byte servoNumber, unsigned int max );
		unsigned int getMaxPosition( byte servoNumber );
		
		// Gestion position
		void setPosition( byte servoNumber, unsigned int position );
		unsigned int getPosition( byte servoNumber );
		
		void setTargetPosition( byte servoNumber, unsigned int target );
		unsigned int getTargetPosition( byte servoNumber );
		
		// Gestion de la vitesse
		void setSpeed( byte servoNumber, unsigned int speed );
		unsigned int getSpeed( byte servoNumber );
		
		// Gestion du couple
		void setMaxTorque( byte servoNumber, unsigned int maxTorque );
		unsigned int getMaxTorque( byte servoNumber );
		unsigned int getTorque( byte servoNumber );
		
	private :
	
		byte _currentServo;
		MyServo _servos[4];
	
		void initTimer1();
		void rearmTimer1( unsigned int code );
		void initTimer2();
		
		void handleNextServo();
		
};

#endif