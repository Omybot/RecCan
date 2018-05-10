#include <MyServo.h>

MyServo::MyServo(){
}

MyServo::~MyServo(){
}

////////////////////////////////////////
// Gestion Initialisation
////////////////////////////////////////

void MyServo::attach( int outputPin, int analogPin ){
	
	_outputPin = outputPin;
	pinMode( _outputPin, OUTPUT );
	
	_analogPin = analogPin;
	pinMode( _analogPin, INPUT );
	
}

////////////////////////////////////////
// Gestion position (déplacement direct)
////////////////////////////////////////

void MyServo::setPosition( float position ){
	_targetPosition = position;
	_position = position;
}

float MyServo::getPosition(){
	return _position;
}

////////////////////////////////////////
// Gestion position (déplacement avec param vitesse et accel)
////////////////////////////////////////

void MyServo::setTargetPosition( float position ){
	_targetPosition = position;
	_speed = 0;
}

float MyServo::getTargetPosition(){
	return _targetPosition;
}

////////////////////////////////////////
// Routine de calcul position suivante (toutes les 5 ms)
////////////////////////////////////////

float MyServo::getNextPosition(){
	
	if( _targetPosition == 0 ){
		_position = 0;
		return _position;
	}
	
	// Calcul de la direction
	float error = _targetPosition - _position;			// Calcul de l'erreur de position

	if( error > 0 ){
		_position += _speed;
		if( _position > _targetPosition ) _position = _targetPosition;
	} else if( error < 0 ){
		_position -= _speed;
		if( _position < _targetPosition ) _position = _targetPosition;
	}
	
	_speed += _accel;									// Calcul nouvelle vitesse
	if( _speed > _maxSpeed ) _speed = _maxSpeed;		// Limite de vitesse
	
	return _position;
	
}

////////////////////////////////////////
// Gestion vitesse
////////////////////////////////////////

void MyServo::setMaxSpeed( float maxSpeed ){
	_maxSpeed = maxSpeed;
}

float MyServo::getMaxSpeed(){
	return _maxSpeed;
}

float MyServo::getSpeed(){
	return _speed;
}

////////////////////////////////////////
// Gestion acceleration
////////////////////////////////////////

void MyServo::setAccel( float accel ){
	_accel = accel;
}

float MyServo::getAccel(){
	return _accel;
}

////////////////////////////////////////
// Gestion pin de sortie
////////////////////////////////////////

void MyServo::setOutputHigh(){
	digitalWrite( _outputPin, HIGH );
}

void MyServo::setOutputLow(){
	digitalWrite( _outputPin, LOW );
}
