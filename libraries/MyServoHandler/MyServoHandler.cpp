#include <MyServoHandler.h>

MyServoHandler::MyServoHandler(){
}

MyServoHandler::~MyServoHandler(){
}

////////////////////////////////////////
// Gestion Initialisation
////////////////////////////////////////

void MyServoHandler::attach(){
	
	_servos[0].attach( 10, A0 );
	_servos[1].attach( 9, A1 );
	_servos[2].attach( 6, A2 );
	_servos[3].attach( 5, A3 );
	
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

void MyServoHandler::setTargetPosition( byte servoNumber, float position ){
	_servos[servoNumber].setTargetPosition( position );
}

float MyServoHandler::getTargetPosition( byte servoNumber ){
	return _servos[servoNumber].getTargetPosition();
}

float MyServoHandler::getPosition( byte servoNumber ){
	return _servos[servoNumber].getPosition();
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
// Gestion vitesse
////////////////////////////////////////

void MyServoHandler::setAccel( byte servoNumber, float accel ){
	_servos[servoNumber].setAccel(accel);
}

float MyServoHandler::getAccel( byte servoNumber ){
	return _servos[servoNumber].getAccel();
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
