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
		
		// Gestion position (déplacement direct)
		void setPosition( byte servoNumber, float target );
		float getPosition( byte servoNumber );
		
		// Gestion position (déplacement avec acceleration et vitesse)
		void setTargetPosition( byte servoNumber, float position );
		float getTargetPosition( byte servoNumber );
		
		// Gestion de la vitesse
		void setMaxSpeed( byte servoNumber, float maxSpeed );
		float getMaxSpeed( byte servoNumber );
		float getSpeed( byte servoNumber );
		
		// Gestion de l'acceleration
		void setAccel( byte servoNumber, float accel );
		float getAccel( byte servoNumber );
		
		// Gestion des trajectoires
		void setTrajectory( byte servoNumber, float targetPosition, float maxSpeed, float accel );
		
		
	private :
	
		byte _currentServo;
		MyServo _servos[4];
	
		void initTimer1();
		void rearmTimer1( float code );
		void initTimer2();
		
		void handleNextServo();
		
};

#endif