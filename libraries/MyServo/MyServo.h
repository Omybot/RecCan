#ifndef MY_SERVO_H
#define MY_SERVO_H

#include <Arduino.h>
#include <EEPROM.h>

enum ServoFunction : byte {
	PositionAsk = 0x01,
	PositionResponse = 0x02,
	PositionSet = 0x03,
	PositionMinAsk = 0x04,
	PositionMinResponse = 0x05,
	PositionMinSet = 0x06,
	PositionMaxAsk = 0x07,
	PositionMaxResponse = 0x08,
	PositionMaxSet = 0x09,
	SpeedLimitAsk = 0x0A,
	SpeedLimitResponse = 0x0B,
	SpeedLimitSet = 0x0C,
	TorqueLimitAsk = 0x0D,
	TorqueLimitResponse = 0x0E,
	TorqueLimitSet = 0x0F,
	TorqueAsk = 0x10,
	TorqueResponse = 0x11,
	AccelerationAsk = 0x12,
	AccelerationResponse = 0x13,
	AccelerationSet = 0x14,
	TargetSet = 0x15,
	TrajectorySet = 0x16,
	DisableOutput = 0x17,
	TorqueAlert = 0x18,
	TestConnection = 0x19
};

const uint16_t DEFAULT_POSITIONMIN = 0;
const uint16_t DEFAULT_POSITIONMAX = 60000;
const float DEFAULT_SPEED_LIMIT = 60000.0;
const float DEFAULT_ACCELERATION = 60000.0;
const unsigned int DEFAULT_TORQUE_LIMIT = 100;	// 100mA Maxi

class MyServo {

	public :

		MyServo();
		~MyServo();

		// Initialisation
		void attach( int outputPin, int analogPin, int servoNum );

		// Gestion pin de sortie
		void setOutputHigh();
		void setOutputLow();

		// Gestion puissance en sortie
		void enableOutput();
		void disableOutput();
		bool isOutputEnable();

		// Parametres servo
		void setPositionMin( uint16_t positionMin );
		void setPositionMax( uint16_t positionMin );
		void setSpeedLimit( float speedLimit );
		void setAcceleration( float acceleration );
		void setTorqueLimit( unsigned int torqueLimit );

		uint16_t getPositionMin();
		uint16_t getPositionMax();
		float getSpeedLimit();
		float getAcceleration();
		unsigned int getTorqueLimit();

		// Mesure du couple
		void updateTorque();
		unsigned int getTorque();

		// Gestion position/vitesse
		void setPosition( float position );
		float getPosition();
		float getSpeed();
		float calcNextPosition();						// Routine de calcul position (doit etre appelée toutes les 20 ms)

	private :

		int _outputPin;
		int _analogPin;
		int _servoNum;

		uint16_t _positionMin;
		uint16_t _positionMax;
		float _speedLimit;
		float _acceleration;
		unsigned int _torqueLimit;

		float _position;
		unsigned int _torque;
		float _speed;

		float _targetPosition;
		float _startPosition;

		float _trajectoryTime;

		bool _enable;

};

#endif
