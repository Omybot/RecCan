#ifndef MY_SERVO_H
#define MY_SERVO_H

#include <Arduino.h>

enum ServoFunction : byte {	PositionAsk = 0x01,
	PositionResponse = 0x02,
	PositionSet = 0x03,
	PositionMinAsk = 0x04,
	PositionMinResponse = 0x05,
	PositionMinSet = 0x06,
	PositionMaxAsk = 0x07,
	PositionMaxResponse = 0x08,
	PositionMaxSet = 0x09,
	SpeedAsk = 0x0A,
	SpeedResponse = 0x0B,
	SpeedSet = 0x0C,
	TorqueMaxAsk = 0x0D,
	TorqueMaxResponse = 0x0E,
	TorqueMaxSet = 0x0F, 
	TorqueCurrentAsk = 0x10,
	TorqueCurrentResponse = 0x11,
	AccelerationAsk = 0x12,
	AccelerationResponse = 0x13,
	AccelerationSet = 0x14,
	TargetSet = 0x15,
	TrajectorySet = 0x16
};

class MyServo {
	
	public :
	
		MyServo();
		~MyServo();
		
		// Initialisation
		void attach( int outputPin, int analogPin );
		
		// Gestion position (déplacement direct)
		void setPosition( float position );
		float getPosition();
		
		// Gestion position (déplacement avec param vitesse et accel)
		void setTargetPosition( float position );
		float getTargetPosition();
		
		// Routine de calcul position suivante (toutes les 5 ms)
		float getNextPosition();
		
		// Gestion vitesse
		void setMaxSpeed( float maxSpeed );
		float getMaxSpeed();
		float getSpeed();
		
		// Gestion vitesse
		void setAccel( float accel );
		float getAccel();
		
		// Gestion couple
		float getTorque();
		void updateTorque();
		
		// Gestion pin de sortie
		void setOutputHigh();
		void setOutputLow();
		
	private :
	
		int _outputPin;
		int _analogPin;
		
		float _targetPosition;
		float _position;
		float _initPosition;
		
		float _maxSpeed = 0xFFFF;
		float _speed = 0;
		float _accel = 0xFFFF;
		
		float _torque;
	
};

#endif