#include <Servo.h>

const int ledPin = 13;
const int servoPin = 9;

Servo myservo;

int pos;

void setup() {

  Serial.begin(115200);
  
  pinMode(13, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(servoPin,OUTPUT);
  
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pos = 75;
  myservo.write(pos);

}

void loop() {

  //while(1){
  //  myservo.write(105);
  //}
  
  while (Serial.available() > 0) {

    char c = Serial.read();
    //if(c == 'E'){
    //  Serial.println("Error");
      //myservo.write(45);
    //} else 
    if(c == '2'){
      digitalWrite(ledPin, HIGH);
      Serial.println("Nord");
      //myservo.write(45);
    } else if(c == '1'){
      Serial.println("Sud");
      digitalWrite(ledPin, LOW);
      //myservo.write(90);
    }

  }

  if( digitalRead(ledPin) && pos < 105 )
    pos++;
  else if( !digitalRead(ledPin) && pos > 50 )
    pos--;
  delay(25);
  myservo.write(pos);

  //Serial.println( pos );
  
}
