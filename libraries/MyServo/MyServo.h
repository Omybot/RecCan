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
		void setPosition( unsigned int position );
		
		void setMinPosition( unsigned int min );
		unsigned int getMinPosition();
		void setMaxPosition( unsigned int max );
		unsigned int getMaxPosition();
		
		unsigned int getPosition();
		unsigned int getNextPosition();
		
		void setTargetPosition( unsigned int target );
		unsigned int getTargetPosition();
		
		// Gestion de la vitesse
		void setSpeed( unsigned int speed );
		unsigned int getSpeed();
		
		// Gestion couple
		void setMaxTorque( unsigned int maxTorque );
		unsigned int getMaxTorque();
		unsigned int getTorque();
		void updateTorque();
		
		// Gestion pin de sortie
		void setOutputHigh();
		void setOutputLow();
		
	private :
	
		int _outputPin;
		int _analogPin;
		
		int _direction;
		
		unsigned int _targetPosition;
		unsigned int _position;
		unsigned int _minPosition = 8000;
		unsigned int _maxPosition = 32000;
		
		unsigned int _speed = 500;
		unsigned int _reverseSpeed = 100;
		
		unsigned int _maxTorque = 1000;
		int _torque;
		
};

#endif