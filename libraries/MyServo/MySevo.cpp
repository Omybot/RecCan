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
	// if( target < _minPosition ) 		_targetPosition = _minPosition;
	// else if( target > _maxPosition ) 	_targetPosition = _maxPosition;
	// else 								_targetPosition = target;
	
	_position = target;
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
