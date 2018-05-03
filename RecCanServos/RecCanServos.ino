#include <EEPROM.h>
#include <MyQueue.h>
#include <SPI.h>
#include "mcp_can.h"

/////////////////////////////////
// Gestion bus CAN
/////////////////////////////////

  #define SPI_CS_PIN 8
  MCP_CAN CAN(SPI_CS_PIN);
  
  // Déclaration structure pour une trame
  struct canFrame{
    unsigned int id;
    unsigned char size;
    unsigned char msg[8];
  };
  
  // Stockage des trames
  volatile MyQueue<canFrame> framesFromCAN( 5 );              // Pile pour stocker les messages provenant du bus CAN
  volatile MyQueue<canFrame> framesToCAN( 5 );                // Pile pour stocker les messages à envoyer sur le bus CAN
  
  // Fonction déclenchée lors reception des trames
  void MCP2515_ISR(){
    while (CAN_MSGAVAIL == CAN.checkReceive()){
      canFrame f;
      CAN.readMsgBuf( &f.size, f.msg );
      f.id = CAN.getCanId();
      framesFromCAN.push( f );                                // Stockage du message provenant du bus CAN
    }
  }

  // Envoi de la dernière trame de la queue
  void handleFramesToCAN(){
    if( framesToCAN.count() > 0 ){
      canFrame f = framesToCAN.pop();
      CAN.sendMsgBuf( f.id, 0, f.size, f.msg );               // Envoi du message sur le bus CAN
    }
  }

/////////////////////////////////
// Fin Gestion bus CAN
/////////////////////////////////

/////////////////////////////////
// Gestion des servos
/////////////////////////////////

  const int servoAnalogPin[4] = {A0,A1,A2,A3};
  const int servoPin[4] = {10,9,6,5};                         // Numéro de pin Arduino pour chaque servo
  
  byte currentServoNumber = 0;
  unsigned int servoPosition[4] = {0,0,0,0};                  // Code qui permet de gérer la pwm pour chaque servo (15999 = 1ms HIGH sur 20ms de periode)
  unsigned int servoTorque[4];

  // Sortie du servo désigné à zero
  void disableServo( byte servoNumber ){
    digitalWrite( servoPin[ servoNumber ], LOW );
  }

  // Gestion de la pwm du servo suivant
  void handleNextServo(){
    currentServoNumber = (currentServoNumber+1) % 4;          // Mise à jour numéro servo suivant
    if( servoPosition[ currentServoNumber ] != 0 ){           // Si un code est désigné
      rearmTimer1( servoPosition[ currentServoNumber ] );     // Réarmement du timer1 pour le code donné
      digitalWrite( servoPin[ currentServoNumber ], HIGH );   // Sortie à 1
    } else {
      rearmTimer1( 1000 );
    }
  }

  void getServoTorque( byte servoNumber ){
    servoTorque[servoNumber] = analogRead( servoAnalogPin[servoNumber] );
  }

/////////////////////////////////
// Fin Gestion des servos
/////////////////////////////////

/////////////////////////////////
// Gestion TIMER 1 => Utilisation comme compte a rebours
/////////////////////////////////

  void initTimer1(){
    
    noInterrupts();                                           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    TCCR1B |= (1 << WGM12);                                   // CTC mode
    TIMSK1 |= (1 << OCIE1A);                                  // enable timer compare interrupt
    interrupts();                                             // enable all interrupts
    
  }
  
  // Timer 1 compare interrupt service routine
  ISR(TIMER1_COMPA_vect){
    
    TCCR1B &= 0b11111000;                                     // Arret du timer1
    disableServo( currentServoNumber );
    handleFramesToCAN();                                      // Dépile les trames pour un envoi sur le bus CAN
    
  }
  
  // Réarmement du timer -> déclenche une interruption ISR(TIMER1_COMPA_vect) au bout de : code * 62,5ns
  void rearmTimer1( int code ){
    
    OCR1A = code;                                             // Compare match register (15999+1)/16MHz = 1ms
    TCNT1  = 0;                                               // Remise à zéro du compteur
    TCCR1B |= (1 << CS10);                                    // Prescaler = 1 -> fclk = 16MHz (62,5ns)
    
  }

/////////////////////////////////
// Gestion TIMER 2 => Interruption toutes les 5ms
/////////////////////////////////

  // Initilisation du time pour déclencher une interruption toutes les 5ms
  void initTimer2(){

    noInterrupts();                                           // disable all interrupts
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;
  
    OCR2A = 77;                                               // Compare match register (77+1)*1024/16MHz ~= 5ms
    TCCR2A |= (1 << WGM21);                                   // CTC mode
    TCCR2B |= (1 << CS22);                                    // Prescaler = 1024 -> fclk = 15,625kHz (64us)
    TCCR2B |= (1 << CS21);
    TCCR2B |= (1 << CS20);
    TIMSK2 |= (1 << OCIE2A);                                  // enable timer compare interrupt
    interrupts();                                             // enable all interrupts
    
  }
  
  // Interruption overflow compteur timer 2
  ISR(TIMER2_COMPA_vect){
    getServoTorque( currentServoNumber );
    handleNextServo();                                        // Gestion des pwm de chaque servo
  }

/////////////////////////////////
// Fin Gestion TIMER 2
/////////////////////////////////

/////////////////////////////////
// Autres Variables Globales
/////////////////////////////////

/////////////////////////////////
// SETUP
/////////////////////////////////
void setup() {

  // Gestion des pins
  for( int i=0; i<4; i++ ) pinMode( servoPin[i], OUTPUT );
  for( int i=0; i<4; i++ ) pinMode( servoAnalogPin[i], INPUT );

  // Initialisation des timers
  initTimer1();                                               // timer1 pret a décompter
  initTimer2();                                               // interruption timer2 toutes les 5ms
  
  // Initialisation CAN
  while (CAN_OK != CAN.begin(CAN_500KBPS)){ delay(10); }

  // Fonction d'interruption quand récéption trame bus CAN
  attachInterrupt(0, MCP2515_ISR, FALLING);

  Serial.begin(115200);

  // Filtre pour ne prendre en compte que les trames désirées
  CAN.init_Mask(0, 0, 0x7ff); 
  CAN.init_Mask(1, 0, 0x7ff); 
  int canId = 1;
  CAN.init_Filt(0, 0, canId);

}

/////////////////////////////////
// LOOP
/////////////////////////////////
void loop() {
  
  if( !framesFromCAN.isEmpty() ){                             // Test si des trames ont été recues

    volatile canFrame f = framesFromCAN.pop();                // Dépilage de la derniere trame
    
    byte frameNumber = f.msg[0];
    byte commandId = f.msg[1];
    byte servoId = f.msg[2];

    Serial.println( "Nouvelle commande " );
    Serial.print( "id : " );
    Serial.println( f.id , HEX );
    Serial.print( "fum : " );
    Serial.println( frameNumber, HEX );
    Serial.print( "cmd : " );
    Serial.println( commandId , HEX );
    Serial.print( "servoid : " );
    Serial.println( servoId , HEX );
    
    canFrame response;
    response.id = f.id;
    response.msg[0] = frameNumber;
    response.msg[1] = commandId;
    response.msg[2] = servoId;
    response.size = 3;

    switch( commandId ){
      case 0 :                                                    // Pilotage en position
          servoPosition[servoId] = f.msg[3] * 0x100 + f.msg[4];   // Récupération valeur position
          Serial.println( servoPosition[servoId] );
          response.msg[3] = servoPosition[servoId] / 0x100;       // Renvoi valeur recue en réponse
          response.msg[4] = servoPosition[servoId] % 0x100;
          response.size += 2;
        break;
      case 1 :                                                    // Retour position
          response.msg[3] = servoPosition[servoId] / 0x100;       // Renvoi position en réponse
          response.msg[4] = servoPosition[servoId] % 0x100;
          response.size += 2;
        break;
      case 2 :                                                    // Retour couple
          response.msg[3] = servoTorque[servoId] / 0x100;         // Renvoi valeur couple en réponse
          response.msg[4] = servoTorque[servoId] % 0x100;
          response.size += 2;
        break;
      //default :
    }

    if( response.id != 0 ){                                       // Test si une trame à envoyer
      framesToCAN.push( response );
    }
    
  }

}


