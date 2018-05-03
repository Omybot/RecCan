#include <MyServoHandler.h>

MyServoHandler::MyServoHandler(){
}

MyServoHandler::~MyServoHandler(){
}

void MyServoHandler::attach(){
	
	_servos[0].attach( 10, A0 );
	_servos[1].attach( 9, A1 );
	_servos[2].attach( 6, A2 );
	_servos[3].attach( 5, A3 );
	
	initTimer1();
	initTimer2();
	
}

void MyServoHandler::timer1Interrupt(){
	_servos[_currentServo].setOutputLow();
}

void MyServoHandler::timer2Interrupt(){
	handleNextServo();
	_servos[_currentServo].updateTorque();
}

void MyServoHandler::setMinPosition( byte servoNumber, unsigned int min ){
	_servos[servoNumber].setMinPosition( min );
}
	
unsigned int MyServoHandler::getMinPosition( byte servoNumber ){
	return _servos[servoNumber].getMinPosition( );
}

void MyServoHandler::setMaxPosition( byte servoNumber, unsigned int max ){
	_servos[servoNumber].setMaxPosition( max );
}
	
unsigned int MyServoHandler::getMaxPosition( byte servoNumber ){
	return _servos[servoNumber].getMaxPosition();
}

void MyServoHandler::setPosition( byte servoNumber, unsigned int position ){
	_servos[servoNumber].setPosition( position );
}

unsigned int MyServoHandler::getPosition( byte servoNumber ){
	return _servos[servoNumber].getPosition();
}

void MyServoHandler::setTargetPosition( byte servoNumber, unsigned int target ){
	_servos[servoNumber].setTargetPosition( target );
}

unsigned int MyServoHandler::getTargetPosition( byte servoNumber ){
	return _servos[servoNumber].getTargetPosition();
}

void MyServoHandler::setSpeed( byte servoNumber, unsigned int speed ){
	_servos[servoNumber].setSpeed(speed);
}

unsigned int MyServoHandler::getSpeed( byte servoNumber ){
	return _servos[servoNumber].getSpeed();
}

void MyServoHandler::setMaxTorque( byte servoNumber, unsigned int maxTorque ){
	_servos[servoNumber].setMaxTorque(maxTorque);
}

unsigned int MyServoHandler::getMaxTorque( byte servoNumber ){
	return _servos[servoNumber].getMaxTorque();
}

unsigned int MyServoHandler::getTorque( byte servoNumber ){
	return _servos[servoNumber].getTorque();
}

void MyServoHandler::initTimer1(){
	noInterrupts();				// disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
	TCCR1B |= (1 << WGM12);		// CTC mode
	TIMSK1 |= (1 << OCIE1A);	// enable timer compare interrupt
	interrupts(); 
}

void MyServoHandler::rearmTimer1( unsigned int code ){
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
	unsigned int newPos = _servos[_currentServo].getNextPosition();
	
	if( newPos != 0 ){
		rearmTimer1( newPos );						// Timer1 comme chronometre pour compter temps sortie servo état haut
		_servos[_currentServo].setOutputHigh();	// Sortie servo à l'état haut
	}
	
}
