#include <MyServo.h>

MyServo::MyServo(){
	_enable = false;
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
	EEPROM.get( 0x10*(_servoNum+1), _positionMin );
	EEPROM.get( 0x10*(_servoNum+1)+2, _positionMax );
	EEPROM.get( 0x10*(_servoNum+1)+4, _speedLimit );
	EEPROM.get( 0x10*(_servoNum+1)+8, _acceleration );
	EEPROM.get( 0x10*(_servoNum+1)+12, _torqueLimit );

	// Sécurité si eeprom pas initialisée
	if( _positionMin == 0xFFFF || isnan(_positionMin) ) setPositionMin( DEFAULT_POSITIONMIN );
	if( _positionMax == 0xFFFF || _positionMax == 0 || isnan(_positionMax) ) setPositionMax( DEFAULT_POSITIONMAX );
	if( _speedLimit == 0xFFFF || _speedLimit == 0 || isnan(_speedLimit) ) setSpeedLimit( DEFAULT_SPEED_LIMIT );
	if( _acceleration == 0xFFFF || _acceleration == 0 || isnan(_acceleration) ) setAcceleration( DEFAULT_ACCELERATION );
	if( _torqueLimit == 0xFFFF || _torqueLimit == 0 || isnan(_torqueLimit)) setTorqueLimit( DEFAULT_TORQUE_LIMIT );

	_speed = _speedLimit;

}

////////////////////////////////////////
// Gestion pin de sortie
////////////////////////////////////////

void MyServo::setOutputHigh(){ digitalWrite( _outputPin, HIGH ); }
void MyServo::setOutputLow(){ digitalWrite( _outputPin, LOW ); }

////////////////////////////////////////
// Gestion puissance en sortie
////////////////////////////////////////
void MyServo::enableOutput(){ _enable = true; }
void MyServo::disableOutput(){ _enable = false; }

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

void MyServo::setTorqueLimit( unsigned int torqueLimit ){
	_torqueLimit = torqueLimit;
	EEPROM.put( 0x10*(_servoNum+1)+12, _torqueLimit);
}

uint16_t MyServo::getPositionMin(){	return _positionMin; }
uint16_t MyServo::getPositionMax(){ return _positionMax; }
float MyServo::getSpeedLimit(){ return _speedLimit; }
float MyServo::getAcceleration(){ return _acceleration; }
unsigned int MyServo::getTorqueLimit(){ return _torqueLimit; }

////////////////////////////////////////
// Mesure du couple
////////////////////////////////////////

void MyServo::updateTorque(){

	_torque = analogRead( _analogPin );

}

unsigned int MyServo::getTorque(){ return _torque; }

////////////////////////////////////////
// Gestion position/vitesse
////////////////////////////////////////

void MyServo::setPosition( float position ){
	enableOutput();
	_targetPosition = position;
	_startPosition = _position;
	_trajectoryTime = 0;
}

float MyServo::getPosition(){
	return _position;
}

float MyServo::getSpeed(){
	return _speed;						//TODO: A changer quand l'acceleration sera ok
}

float MyServo::calcNextPosition(){

	_trajectoryTime += 0.02;

	if( _targetPosition > _startPosition ){

		_position = _startPosition + _speedLimit * _trajectoryTime;
		if( _position > _targetPosition ) _position = _targetPosition;

	} else if( _targetPosition < _startPosition ){

		_position = _startPosition - _speedLimit * _trajectoryTime;
		if( _position < _targetPosition ) _position = _targetPosition;

	}

	// Limite de position
	if( _position > _positionMax ) _position = _positionMax;
	if( _position < _positionMin ) _position = _positionMin;

	if( !_enable ) return 0.0;

	return _position;

}
