#include <MyServo.h>

MyServo::MyServo(){
}

MyServo::~MyServo(){
}

////////////////////////////////////////
// Initialisation
////////////////////////////////////////

void MyServo::attach( int outputPin, int analogPin ){
	
	_outputPin = outputPin;
	pinMode( _outputPin, OUTPUT );
	
	_analogPin = analogPin;
	pinMode( _analogPin, INPUT );
	
}

////////////////////////////////////////
// Gestion pin de sortie
////////////////////////////////////////

void MyServo::setOutputHigh(){ digitalWrite( _outputPin, HIGH ); }
void MyServo::setOutputLow(){ digitalWrite( _outputPin, LOW ); }

////////////////////////////////////////
// Parametres servo
////////////////////////////////////////

void MyServo::setPositionMin( uint16_t positionMin ){ _positionMin = positionMin; }
void MyServo::setPositionMax( uint16_t positionMax ){ _positionMax = positionMax; }
void MyServo::setSpeedLimit( float speedLimit ){ _speedLimit = speedLimit; }
void MyServo::setAcceleration( float acceleration ){ _acceleration = acceleration; }
void MyServo::setTorqueLimit( float torqueLimit ){ _torqueLimit = torqueLimit; }

uint16_t MyServo::getPositionMin(){ return _positionMin; }
uint16_t MyServo::getPositionMax(){ return _positionMax; }
float MyServo::getSpeedLimit(){ return _speedLimit; }
float MyServo::getAcceleration(){ return _acceleration; }
float MyServo::getTorqueLimit(){ return _torqueLimit; }

////////////////////////////////////////
// Mesure du couple
////////////////////////////////////////

void MyServo::updateTorque(){

	float voltage = analogRead( _analogPin ) * 5.0 / 1024.0;
	_torque = voltage * 1000;						// Ici facteur de conversion U->I(mA) carte éléctronique

}

float MyServo::getTorque(){ return _torque; }

////////////////////////////////////////
// Gestion position/vitesse
////////////////////////////////////////

void MyServo::setPosition( float position ){
	_targetPosition = position;
}

float MyServo::getPosition(){
	return _position;
}

float MyServo::getSpeed(){
	return _speed;
}

float MyServo::calcNextPosition(){

	_position = _targetPosition;

	return _position;
}

