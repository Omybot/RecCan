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

		// Parametres servos
		void setPositionMin( byte servoNumber, uint16_t positionMin );
		void setPositionMax( byte servoNumber, uint16_t positionMax );
		void setSpeedLimit( byte servoNumber, float speedLimit );
		void setAcceleration( byte servoNumber, float acceleration );
		void setTorqueLimit( byte servoNumber, float torqueLimit );

		uint16_t getPositionMin( byte servoNumber );
		uint16_t getPositionMax( byte servoNumber );
		float getSpeedLimit( byte servoNumber );
		float getAcceleration( byte servoNumber );
		float getTorqueLimit( byte servoNumber );

		// Gestion puissance en sortie
		void enableOutput( byte servoNumber );
		void disableOutput( byte servoNumber );

		// Gestion position/trajectoire
		void setPosition( byte servoNumber, float position );
		void setTrajectory( byte servoNumber, float position, float speedLimit, float acceleration );

		// Infos servo
		float getPosition( byte servoNumber );
		float getSpeed( byte servoNumber );
		float getTorque( byte servoNumber );

		// Gestion des interruptions
		void timer1Interrupt();
		void timer2Interrupt();

	private :

		byte _currentServo;
		MyServo _servos[4];

		void initTimer1();
		void rearmTimer1( float code );
		void initTimer2();

		void handleNextServo();

};

#endif
