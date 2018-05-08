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

void MyServo::setMinPosition( unsigned int min ){
	_minPosition = min;
}
	
unsigned int MyServo::getMinPosition(){
	return _minPosition;
}

void MyServo::setMaxPosition( unsigned int max ){
	_maxPosition = max;
}

unsigned int MyServo::getMaxPosition(){
	return _maxPosition;
}
	
void MyServo::setPosition( unsigned int position ){
	
	if( position == 0 ) 				_position = position;		// Force la desactivation du servo avec le code 0
	else if( position < _minPosition ) 	_position = _minPosition;
	else if( position > _maxPosition ) 	_position = _maxPosition;
	else 								_position = position;
	_targetPosition = _position;
	
}

unsigned int MyServo::getPosition(){
	return _position;
}

unsigned int MyServo::getNextPosition(){
	
	// Calcul de la direction
	long error = (long)_targetPosition - (long)_position;			// Calcul de l'erreur de position
	if( error > 0 ) 		_direction = 1;							// position très inférieure à la cible
	else if( error < 0 ) 	_direction = -1;						// position supérieure à la cible
	else if( error == 0 )	return _position;
	
	// Calcul de la position suivante
	if( _torque > _maxTorque )	_position -= _direction*_speed;
	else						_position += _direction*_speed;
	
	// Vérification dépassement de la cible
	if( ( _direction == 1 && _position > _targetPosition ) || ( _direction == -1 && _position < _targetPosition ) )
		_position = _targetPosition;
	
	//Vérification des limites
	if( _position < _minPosition ) 		_position = _minPosition;	// Vérif des limites
	else if( _position > _maxPosition ) _position = _maxPosition;
	
	return _position;
	
}

void MyServo::setTargetPosition( unsigned int target ){

	// Vérification des limites
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
	
void MyServo::setMaxTorque( unsigned int maxTorque ){
	_maxTorque = maxTorque;
}

unsigned int MyServo::getMaxTorque(){
	return _maxTorque;
}

unsigned int MyServo::getTorque(){
	return _torque;
}

void MyServo::updateTorque(){
	_torque = analogRead( _analogPin );
}

void MyServo::setOutputHigh(){
	digitalWrite( _outputPin, HIGH );
}

void MyServo::setOutputLow(){
	digitalWrite( _outputPin, LOW );
}
