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
	
	_targetPosition = position;						// Mise à jour de la valeur position cible
	_initPosition = _position;						// Enregistrement position initiale
	
	_speed = 0;										// Vitesse de départ à zéro
	
	
	//_ticksToDo = _targetPosition - _position;		// Calcul de la distance à parcourir
	
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
	
	// Calcul de la distance parcourue
	float distanceToDo = abs( _targetPosition - _position );
	float traveledDistance = abs( _position - _initPosition );
	
	int direction;
	if( ( _targetPosition - _position ) > 0 ) direction = 1;
	else if( ( _targetPosition - _position ) < 0 ) direction = -1;
	else direction = 0;
	
	if( traveledDistance >= distanceToDo ){					// Si j'ai parcouru plus de la moitié du chemin
		if( abs(_accel) >= abs(_speed) ) _speed = _accel;
		else _speed -= _accel;									// Il faut diminuer la vitesse
		if( distanceToDo < abs( _speed )) _speed = distanceToDo;
	} else {
		_speed += _accel;									// Il faut augmenter la vitesse
	}
	
	// Mise à jour position + verif vmax
	if( _speed > _maxSpeed ) 
		_position += _maxSpeed * direction;
	else 
		_position += _speed * direction;
	
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
// Gestion couple
////////////////////////////////////////

float MyServo::getTorque(){
	return _torque;
}

void MyServo::updateTorque(){
	_torque = analogRead( _analogPin );
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
