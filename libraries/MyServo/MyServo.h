#ifndef MY_SERVO_H
#define MY_SERVO_H

#include <Arduino.h>

class MyServo {
	
	public :
	
		MyServo();
		~MyServo();
		
		// Initialisation
		void attach( int outputPin, int analogPin );
		
		// Parametres
		void setPositionRange( unsigned int min, unsigned int max );
		
		// Gestion position
		void setPosition( unsigned int position );
		unsigned int getPosition();
		unsigned int getNextPosition();
		
		void setTargetPosition( unsigned int target );
		unsigned int getTargetPosition();
		
		// Gestion de la vitesse
		void setSpeed( unsigned int speed );
		unsigned int getSpeed();
		
		// Gestion couple
		unsigned int getTorque();
		
		// Gestion pin de sortie
		void setOutputHigh();
		void setOutputLow();
		
	private :
	
		int _outputPin;
		int _analogPin;
		
		unsigned int _targetPosition;
		unsigned int _position;
		unsigned int _minPosition = 8000;
		unsigned int _maxPosition = 32000;
		
		int _speed = 500;
		
		int _torque;
		
};

#endif