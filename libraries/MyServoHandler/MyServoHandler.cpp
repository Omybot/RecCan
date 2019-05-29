#include <MyServoHandler.h>

//#define OUT_PIN_SERVO_0 10
//#define OUT_PIN_SERVO_1 9
//#define OUT_PIN_SERVO_2 6
//#define OUT_PIN_SERVO_3 5

//#define ANALOG_PIN_SERVO_0 A0
//#define ANALOG_PIN_SERVO_1 A1
//#define ANALOG_PIN_SERVO_2 A2
//#define ANALOG_PIN_SERVO_3 A3

#define OUT_PIN_SERVO_0 5
#define OUT_PIN_SERVO_1 6
#define OUT_PIN_SERVO_2 7
#define OUT_PIN_SERVO_3 8
#define ANALOG_PIN_SERVO_0 A2
#define ANALOG_PIN_SERVO_1 A3
#define ANALOG_PIN_SERVO_2 A4
#define ANALOG_PIN_SERVO_3 A5

MyServoHandler::MyServoHandler(){
}

MyServoHandler::~MyServoHandler(){
}

////////////////////////////////////////
// Initialisation
////////////////////////////////////////

void MyServoHandler::attach(){
	_servos[0].attach(OUT_PIN_SERVO_0, ANALOG_PIN_SERVO_0, 0);
	_servos[1].attach(OUT_PIN_SERVO_1, ANALOG_PIN_SERVO_1, 1);
	_servos[2].attach(OUT_PIN_SERVO_2, ANALOG_PIN_SERVO_2, 2);
	_servos[3].attach(OUT_PIN_SERVO_3, ANALOG_PIN_SERVO_3, 3);

	initTimer1();
	initTimer2();
}

////////////////////////////////////////
// Parametres servos
////////////////////////////////////////

void MyServoHandler::setPositionMin( byte servoNumber, uint16_t positionMin ){
	_servos[servoNumber].setPositionMin( positionMin );
}

void MyServoHandler::setPositionMax( byte servoNumber, uint16_t positionMax ){
	_servos[servoNumber].setPositionMax( positionMax );
}

void MyServoHandler::setSpeedLimit( byte servoNumber, float speedLimit ){
	_servos[servoNumber].setSpeedLimit( speedLimit );
}

void MyServoHandler::setAcceleration( byte servoNumber, float acceleration ){
	_servos[servoNumber].setAcceleration( acceleration );
}

void MyServoHandler::setTorqueLimit( byte servoNumber, unsigned int torqueLimit ){
	_servos[servoNumber].setTorqueLimit( torqueLimit );
}

uint16_t MyServoHandler::getPositionMin( byte servoNumber ){
	return _servos[servoNumber].getPositionMin();
}

uint16_t MyServoHandler::getPositionMax( byte servoNumber ){
	return _servos[servoNumber].getPositionMax();
}

float MyServoHandler::getSpeedLimit( byte servoNumber ){
	return _servos[servoNumber].getSpeedLimit();
}

float MyServoHandler::getAcceleration( byte servoNumber ){
	return _servos[servoNumber].getAcceleration();
}

unsigned int MyServoHandler::getTorqueLimit( byte servoNumber ){
	return _servos[servoNumber].getTorqueLimit();
}

////////////////////////////////////////
// Gestion puissance de sortie
////////////////////////////////////////
void MyServoHandler::enableOutput( byte servoNumber ){
	_servos[servoNumber].enableOutput();
}

void MyServoHandler::disableOutput( byte servoNumber ){
	_servos[servoNumber].disableOutput();
}

////////////////////////////////////////
// Gestion position/trajectoire
////////////////////////////////////////

void MyServoHandler::setPosition( byte servoNumber, float position ){
	_servos[servoNumber].setPosition( position );
}

void MyServoHandler::setTrajectory( byte servoNumber, float position, float speedLimit, float acceleration ){
	_servos[servoNumber].setSpeedLimit( speedLimit );
	_servos[servoNumber].setAcceleration( acceleration );
	_servos[servoNumber].setPosition( position );
}

////////////////////////////////////////
// Infos servo
////////////////////////////////////////

float MyServoHandler::getPosition( byte servoNumber ){
	return _servos[servoNumber].getPosition();
}

float MyServoHandler::getSpeed( byte servoNumber ){
	return _servos[servoNumber].getSpeed();
}

unsigned int MyServoHandler::getTorque( byte servoNumber ){
	return _servos[servoNumber].getTorque();
}

////////////////////////////////////////
// Gestion des interruptions
////////////////////////////////////////

void MyServoHandler::timer1Interrupt(){
	_servos[_currentServo].setOutputLow();
	TCCR1B &= 0b11111000; 							// Arret du timer
	//_servos[_currentServo].updateTorque();			// Mesure du couple
}

void MyServoHandler::timer2Interrupt(){
	handleNextServo();
}

////////////////////////////////////////
// Private
////////////////////////////////////////


void MyServoHandler::initTimer1(){
	noInterrupts();						// disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
	TCCR1B |= (1 << WGM12);				// CTC mode
	TIMSK1 |= (1 << OCIE1A);			// enable timer compare interrupt
	interrupts();
}

void MyServoHandler::rearmTimer1( float code ){
	OCR1A = code;						// Compare match register (15999+1)/16MHz = 1ms
	TCNT1 = 0;							// Remise à zéro du compteur
	TCCR1B |= (1 << CS10);				// Prescaler = 1 -> fclk = 16MHz (62,5ns)
}

void MyServoHandler::initTimer2(){
	noInterrupts();						// disable all interrupts
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2  = 0;
	OCR2A = 77;							// Compare match register (77+1)*1024/16MHz ~= 5ms
	TCCR2A |= (1 << WGM21);				// CTC mode
	TCCR2B |= (1 << CS22);				// Prescaler = 1024 -> fclk = 15,625kHz (64us)
	TCCR2B |= (1 << CS21);
	TCCR2B |= (1 << CS20);
	TIMSK2 |= (1 << OCIE2A);			// enable timer compare interrupt
	interrupts();
}

void MyServoHandler::handleNextServo(){

	_currentServo = (_currentServo+1) % 4;						// Determine le servo suivant

	float newPos = _servos[_currentServo].calcNextPosition();	// Calcul la nouvelle position
	_servos[_currentServo].updateTorque();			// Mesure du couple

	if( newPos != 0 ){
		rearmTimer1( newPos );									// Timer1 comme chronometre pour compter temps sortie servo état haut
		_servos[_currentServo].setOutputHigh();					// Sortie servo à l'état haut
	}

}
