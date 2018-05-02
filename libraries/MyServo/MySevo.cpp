#include <MyServo.h>

MyServo::MyServo(){
}

MyServo::~MyServo(){
}

void MyServo::attach( int outputPin, int analogPin ){
	
	_outputPin = outputPin;
	pinMode( _outputPin, OUTPUT );
	
	_analogPin = analogPin;
	pinMode( _analogPin, INPUT );
	
}

void MyServo::setPositionRange( unsigned int min, unsigned int max ){
	_minPosition = min;
	_maxPosition = max;
}
	
void MyServo::setPosition( unsigned int position ){
	if( position < _minPosition ) 		_position = _minPosition;
	else if( position > _maxPosition ) 	_position = _maxPosition;
	else 								_position = position;
	_targetPosition = _position;
}

unsigned int MyServo::getPosition(){
	return _position;
}

unsigned int MyServo::getNextPosition(){
	
	long error = (long)_targetPosition - (long)_position;			// Calcul de l'erreur de position
	
	if( error > _speed )			_position += _speed;			// position très inférieure à la cible
	else if( error <  -_speed )		_position -= _speed;			// position très supérieure à la cible
	else _position = 				_targetPosition;				// position proche de la cible
	
	if( _position < _minPosition ) 		_position = _minPosition;	// Vérif des limites
	else if( _position > _maxPosition ) _position = _maxPosition;
	
	return _position;
	
}

void MyServo::setTargetPosition( unsigned int target ){
	if( target < _minPosition ) 		_targetPosition = _minPosition;
	else if( target > _maxPosition ) 	_targetPosition = _maxPosition;
	else 								_targetPosition = target;
}

unsigned int MyServo::getTargetPosition(){
	return _targetPosition;
}

void MyServo::setSpeed( unsigned int speed ){
	_speed = speed;
}

unsigned int MyServo::getSpeed(){
	return _speed;
}

unsigned int MyServo::getTorque(){
	_torque = analogRead( _analogPin );
	return _torque;
}

void MyServo::setOutputHigh(){
	digitalWrite( _outputPin, HIGH );
}

void MyServo::setOutputLow(){
	digitalWrite( _outputPin, LOW );
}
