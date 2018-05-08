#include <MyServoHandler.h>

MyServoHandler servos;

ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

void setup() {

  Serial.begin(115200);

  servos.attach();

  //servos.setMinPosition( 0, 10000 );
  //servos.setMaxPosition( 0, 30000 );
  
//  servos.setMinPosition( 1, 10000 );
//  servos.setMaxPosition( 1, 30000 );
//  servos.setMaxTorque( 1, 75 );

}

unsigned long time;
int stepTime = 2000;
bool dir;

void loop(){

  if( millis() > time + stepTime ){
    time += stepTime;
    
    if( dir ){
      dir = false;
      
      servos.setPosition( 0, 10000 );
      
//      servos.setSpeed( 1, 300 );
//      servos.setTargetPosition( 1, 10000 );
      
    } else {
      dir = true;
      
      servos.setPosition( 0, 0 );
      
//      servos.setSpeed( 1, 150 );
//      servos.setTargetPosition( 1, 30000 );
    }
    
  }
  
  Serial.print( servos.getPosition(0) );
  Serial.print( "\t" );
  Serial.print( servos.getTargetPosition(0) );
  Serial.print( "\t" );
  Serial.print( servos.getTorque(0) );
  Serial.print( "\t" );
  Serial.print( servos.getSpeed(0) );
  Serial.print( "\t" );
  Serial.print( servos.getPosition(1) );
  Serial.print( "\t" );
  Serial.print( servos.getTargetPosition(1) );
  Serial.print( "\t" );
  Serial.print( servos.getTorque(1) );
  Serial.print( "\t" );
  Serial.print( servos.getSpeed(1) );
    Serial.println();
    delay(50);

}
