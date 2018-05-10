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
		void setTargetPosition( float position );
		float getTargetPosition();
		float getPosition();
		float getNextPosition();
		
		// Gestion vitesse
		void setMaxSpeed( float maxSpeed );
		float getMaxSpeed();
		float getSpeed();
		
		// Gestion vitesse
		void setAccel( float accel );
		float getAccel();
		
		// Gestion pin de sortie
		void setOutputHigh();
		void setOutputLow();
		
	private :
	
		int _outputPin;
		int _analogPin;
		
		float _targetPosition;
		float _position;
		
		float _maxSpeed = 0xFFFF;
		float _speed = 0;
		float _accel = 0xFFFF;
		
};

#endif