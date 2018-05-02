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
		
		// Gestion position
		void setPosition( byte servoNumber, unsigned int position );
		unsigned int getPosition( byte servoNumber );
		
		// Gestion du couple
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