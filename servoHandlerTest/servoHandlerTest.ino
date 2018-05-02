#include <MyServoHandler.h>

MyServoHandler servos;

ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

void setup() {

  Serial.begin(115200);

  servos.attach();
  
  servos.setRange( 0, 8000, 34000 );

  servos.setPosition( 0, 16000 );

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
    } else {
      dir = true;
      servos.setPosition( 0, 30000 );
    }
    
  }
  
  Serial.print( dir );
  Serial.print( "     " );
  Serial.print( servos.getPosition( 0 ) );
  Serial.print( "     " );
  Serial.println( servos.getTorque(0) );
  delay(20);

}
