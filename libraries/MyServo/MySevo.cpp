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

void MyServo::setPosition( unsigned int target ){
	 if( target < _minPosition ) 		_position = _minPosition;
	 else if( target > _maxPosition ) 	_position = _maxPosition;
	 else 								_position = target;
}

void MyServo::setPositionRange( unsigned int min, unsigned int max ){
	_minPosition = min;
	_maxPosition = max;
}

unsigned int MyServo::getPosition(){
	return _position;
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
