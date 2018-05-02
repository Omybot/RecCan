#ifndef MY_SERVO_H
#define MY_SERVO_H

#include <Arduino.h>

class MyServo {
	
	public :
	
		MyServo();
		~MyServo();
		
		// Initialisation
		void attach( int outputPin, int analogPin );
		
		// Gestion position
		void setPosition( unsigned int target );
		void setPositionRange( unsigned int min, unsigned int max );
		unsigned int getPosition();
		
		// Gestion couple
		unsigned int getTorque();
		
		// Gestion pin de sortie
		void setOutputHigh();
		void setOutputLow();
		
	private :
	
		int _outputPin;
		int _analogPin;
		
		unsigned int _position;
		unsigned int _minPosition = 8000;
		unsigned int _maxPosition = 32000;
		
		unsigned int _torque;
		
};

#endif