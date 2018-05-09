#include <EEPROM.h>
#include <MyQueue.h>
#include <SPI.h>
#include "mcp_can.h"
#include <MyServoHandler.h>
#include <MyQueue.h>

/////////////////////////////////
// Gestion bus CAN
/////////////////////////////////

#define CAN_ID  4

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
// Gestion des interruptions
/////////////////////////////////

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
  handleFramesToCAN(); 
}

/////////////////////////////////
// Gestion écran OLED
/////////////////////////////////

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_SSD1306 display(-1);                         // Instanciation de l'écran

// Mise à jour du score
void updateScore( int score ){
  
  display.clearDisplay();
  display.setCursor(0,0);                             // On commence dans le coin haut gauche
  display.println( "SCORE" );
  display.print( " " );
  display.println( score );
  display.display();

  score = 0;
  
}

// Initialisation Ecran OLED
void OLEDBegin(){
  
  display.begin();
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0,0);                             // On commence dans le coin haut gauche

  updateScore( 999 );
  
}    


int score = 0;

/////////////////////////////////
// SETUP
/////////////////////////////////
void setup() {

  // Gestion de l'écran
  OLEDBegin();
  
  // Initialisation des timers
  initTimer2();                                               // interruption timer2 toutes les 5ms
  
  // Initialisation CAN
  while (CAN_OK != CAN.begin(CAN_500KBPS)){ delay(10); }
  
  // Fonction d'interruption quand récéption trame bus CAN
  attachInterrupt(0, MCP2515_ISR, FALLING);

  // Filtre pour ne prendre en compte que les trames désirées
  CAN.init_Mask(0, 0, 0x7ff); 
  CAN.init_Mask(1, 0, 0x7ff); 
  CAN.init_Filt(0, 0, CAN_ID);    // Ne prend en compte que les trames d'identifiant CAN_ID

}

/////////////////////////////////
// LOOP
/////////////////////////////////
void loop() {

  if( score != 0 ) updateScore( score );
  
  if( !framesFromCAN.isEmpty() ){                             // Test si des trames ont été recues
    
    volatile canFrame f = framesFromCAN.pop();                // Dépilage de la derniere trame
    
    byte frameNumber = f.msg[0];
    byte command = f.msg[1];
    
    canFrame response;
    response.id = f.id;
    response.msg[0] = frameNumber;
    for( int i=1; i<7; i++ ) response.msg[i] = 0x00;
    response.size = 8;
    
    response.msg[1] = command;
    
    switch( command ){

      // Gestion position
      case 0xA0 :     // Envoi position cible
          score = f.msg[3] * 0x100 + f.msg[4];
          response.msg[3] = score / 0x100;    // [MSB] position
          response.msg[4] = score % 0x100;    // [LSB] position
        break;

    }

    response.msg[7] = (response.id / 0x100) ^ (response.id % 0x100);
    for( int i=0; i<7; i++ ) response.msg[7] ^= response.msg[i];
    
    // Retour réponse
    framesToCAN.push( response );
    
  }

}


