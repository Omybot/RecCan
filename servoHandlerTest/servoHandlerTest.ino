#include <MyServoHandler.h>

MyServoHandler servos;

ISR(TIMER1_COMPA_vect){ servos.timer1Interrupt(); }
ISR(TIMER2_COMPA_vect){ servos.timer2Interrupt(); }

void setup() {

  Serial.begin(115200);

  servos.attach();

  servos.setTargetPosition( 0, 10000 );
  delay(2000); 

//  servos.setTrajectory( 0, 30000, maxSpeed, accel );

}

unsigned long time;
int stepTime = 4000;
bool dir;

unsigned long time2;
int stepTime2 = 20;

int maxSpeed = 10000;
int accel = 10;

void loop(){

  if( millis() > time + stepTime ){
    time += stepTime;
    
    if( dir ){
      dir = false;

      Serial.println("setpos 10000");
      //servos.setTargetPosition( 0, 10000 );
      servos.setTrajectory( 0, 10000, maxSpeed, accel );
      
    } else {
      dir = true;
      
      Serial.println("setpos 30000");
      //servos.setPosition( 0, 30000 );
      servos.setTrajectory( 0, 30000, maxSpeed, accel );
      
    }
    
  }
  
  if( millis() > time2 + stepTime2 ){
    time2 += stepTime2;
    
    Serial.print( servos.getPosition(0) , 0);
    Serial.print( "/" );
    Serial.print( servos.getTargetPosition(0) , 0);
    Serial.print( "\t" );
    Serial.print( servos.getSpeed(0) , 0 );
    Serial.print( "/" );
    Serial.print( servos.getMaxSpeed(0) , 0 );
    Serial.print( "\t" );
    Serial.print( servos.getTorque(0) , 0 );
    Serial.println();
    
  }

}
