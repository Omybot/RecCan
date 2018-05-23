#include <MyServoHandler.h>

#define OUT_PIN_SERVO_0 10
#define OUT_PIN_SERVO_1 9
#define OUT_PIN_SERVO_2 6
#define OUT_PIN_SERVO_3 5

#define ANALOG_PIN_SERVO_0 A0
#define ANALOG_PIN_SERVO_1 A1
#define ANALOG_PIN_SERVO_2 A2
#define ANALOG_PIN_SERVO_3 A3

MyServoHandler::MyServoHandler(){
}

MyServoHandler::~MyServoHandler(){
}

////////////////////////////////////////
// Gestion Initialisation
////////////////////////////////////////

void MyServoHandler::attach(){
	_servos[0].attach(OUT_PIN_SERVO_0, ANALOG_PIN_SERVO_0);
	_servos[1].attach(OUT_PIN_SERVO_1, ANALOG_PIN_SERVO_1);
	_servos[2].attach(OUT_PIN_SERVO_2, ANALOG_PIN_SERVO_2);
	_servos[3].attach(OUT_PIN_SERVO_3, ANALOG_PIN_SERVO_3);
	
	initTimer1();
	initTimer2();
}

////////////////////////////////////////
// Gestion des interruptions
////////////////////////////////////////

void MyServoHandler::timer1Interrupt(){
	_servos[_currentServo].setOutputLow();
	TCCR1B &= 0b11111000; 							// Arret du timer
}

void MyServoHandler::timer2Interrupt(){
	handleNextServo();
}

////////////////////////////////////////
// Gestion position
////////////////////////////////////////

void MyServoHandler::setPosition( byte servoNumber, float target ){
	_servos[servoNumber].setPosition( target );
}

float MyServoHandler::getPosition( byte servoNumber ){
	return _servos[servoNumber].getPosition();
}

void MyServoHandler::setTargetPosition( byte servoNumber, float position ){
	_servos[servoNumber].setTargetPosition( position );
}

float MyServoHandler::getTargetPosition( byte servoNumber ){
	return _servos[servoNumber].getTargetPosition();
}


////////////////////////////////////////
// Gestion vitesse
////////////////////////////////////////

void MyServoHandler::setMaxSpeed( byte servoNumber, float maxSpeed ){
	_servos[servoNumber].setMaxSpeed(maxSpeed);
}

float MyServoHandler::getMaxSpeed( byte servoNumber ){
	return _servos[servoNumber].getMaxSpeed();
}

float MyServoHandler::getSpeed( byte servoNumber ){
	return _servos[servoNumber].getSpeed();
}

////////////////////////////////////////
// Gestion acceleration
////////////////////////////////////////

void MyServoHandler::setAccel( byte servoNumber, float accel ){
	_servos[servoNumber].setAccel(accel);
}

float MyServoHandler::getAccel( byte servoNumber ){
	return _servos[servoNumber].getAccel();
}

////////////////////////////////////////
// Gestion trajectoires
////////////////////////////////////////

void MyServoHandler::setTrajectory( byte servoNumber, float targetPosition, float maxSpeed, float accel ){
	_servos[servoNumber].setMaxSpeed(maxSpeed);
	_servos[servoNumber].setAccel(accel);
	_servos[servoNumber].setTargetPosition( targetPosition );
}

////////////////////////////////////////
// Private
////////////////////////////////////////


void MyServoHandler::initTimer1(){
	noInterrupts();				// disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
	TCCR1B |= (1 << WGM12);		// CTC mode
	TIMSK1 |= (1 << OCIE1A);	// enable timer compare interrupt
	interrupts(); 
}

void MyServoHandler::rearmTimer1( float code ){
	OCR1A = code;				// Compare match register (15999+1)/16MHz = 1ms
	TCNT1 = 0;					// Remise à zéro du compteur
	TCCR1B |= (1 << CS10);		// Prescaler = 1 -> fclk = 16MHz (62,5ns)
}

void MyServoHandler::initTimer2(){
	noInterrupts();				// disable all interrupts
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2  = 0;
	OCR2A = 77;					// Compare match register (77+1)*1024/16MHz ~= 5ms
	TCCR2A |= (1 << WGM21);		// CTC mode
	TCCR2B |= (1 << CS22);		// Prescaler = 1024 -> fclk = 15,625kHz (64us)
	TCCR2B |= (1 << CS21);
	TCCR2B |= (1 << CS20);
	TIMSK2 |= (1 << OCIE2A);	// enable timer compare interrupt
	interrupts();  
}

void MyServoHandler::handleNextServo(){
	
	// Determine le servo suivant
	_currentServo = (_currentServo+1) % 4;
	
	// Calcul la nouvelle position
	float newPos = _servos[_currentServo].getNextPosition();
	
	if( newPos != 0 ){
		rearmTimer1( newPos );						// Timer1 comme chronometre pour compter temps sortie servo état haut
		_servos[_currentServo].setOutputHigh();	// Sortie servo à l'état haut
	}
	
}
