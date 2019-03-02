#include <MyServo.h>

MyServo::MyServo(){
}

MyServo::~MyServo(){
}

////////////////////////////////////////
// Initialisation
////////////////////////////////////////

void MyServo::attach( int outputPin, int analogPin, int servoNum ){
	
	_outputPin = outputPin;
	pinMode( _outputPin, OUTPUT );
	
	_analogPin = analogPin;
	pinMode( _analogPin, INPUT );

	_servoNum = servoNum;

	// Récupération des parametres depuis l'EEPROM
	EEPROM.get( 0x10*(_servoNum+1), _positionMin);
	EEPROM.get( 0x10*(_servoNum+1)+2, _positionMax);
	EEPROM.get( 0x10*(_servoNum+1)+4, _speedLimit);
	EEPROM.get( 0x10*(_servoNum+1)+8, _acceleration);
	EEPROM.get( 0x10*(_servoNum+1)+12, _torqueLimit);

	// Sécurité si eeprom pas initialisée
	if( _positionMin >= 0xFFFF ){ _positionMin = 0; EEPROM.put( 0x10*(_servoNum+1), 0 ); }
	if( _positionMax == 0xFFFF ){ _positionMax = 0; EEPROM.put( 0x10*(_servoNum+1)+2, 0 ); }
	if( _speedLimit == 0xFFFF ){ _speedLimit = 0; EEPROM.put( 0x10*(_servoNum+1)+4, 0 ); }
	if( _acceleration == 0xFFFF ){ _acceleration = 0; EEPROM.put( 0x10*(_servoNum+1)+8, 0 ); }
	if( _torqueLimit == 0xFFFF ){ _torqueLimit = 0; EEPROM.put( 0x10*(_servoNum+1)+12, 0 ); }
	
}

////////////////////////////////////////
// Gestion pin de sortie
////////////////////////////////////////

void MyServo::setOutputHigh(){ digitalWrite( _outputPin, HIGH ); }
void MyServo::setOutputLow(){ digitalWrite( _outputPin, LOW ); }

////////////////////////////////////////
// Parametres servo
////////////////////////////////////////

void MyServo::setPositionMin( uint16_t positionMin ){ 
	_positionMin = positionMin; 
	EEPROM.put( 0x10*(_servoNum+1), _positionMin);
}

void MyServo::setPositionMax( uint16_t positionMax ){ 
	_positionMax = positionMax; 
	EEPROM.put( 0x10*(_servoNum+1)+2, _positionMax);
}

void MyServo::setSpeedLimit( float speedLimit ){ 
	_speedLimit = speedLimit; 
	EEPROM.put( 0x10*(_servoNum+1)+4, _speedLimit);
}

void MyServo::setAcceleration( float acceleration ){ 
	_acceleration = acceleration; 
	EEPROM.put( 0x10*(_servoNum+1)+8, _acceleration);
}

void MyServo::setTorqueLimit( float torqueLimit ){
	_torqueLimit = torqueLimit; 
	EEPROM.put( 0x10*(_servoNum+1)+12, _torqueLimit);
}

uint16_t MyServo::getPositionMin(){	return _positionMin; }
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

