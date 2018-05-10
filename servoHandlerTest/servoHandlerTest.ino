#include <MyServoHandler.h>

MyServoHandler servos;

ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

void setup() {

  Serial.begin(115200);

  servos.attach();

  servos.setSpeed( 0, 300 );

}

unsigned long time;
int stepTime = 1000;
bool dir;

unsigned long time2;
int stepTime2 = 100;

void loop(){

  if( millis() > time + stepTime ){
    time += stepTime;
    
    if( dir ){
      dir = false;

      Serial.println("setpos 10000");
      servos.setPosition( 0, 10000 );
      
    } else {
      dir = true;
      
      Serial.println("setpos 30000");
      servos.setPosition( 0, 30000 );
      
    }
    
  }
  
  if( millis() > time2 + stepTime2 ){
    time2 += stepTime2;
    
    Serial.print( servos.getPosition(0) );
    Serial.print( "\t" );
    Serial.print( servos.getSpeed(0) );
    Serial.println();
    
  }

}
