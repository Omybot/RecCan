#include <EEPROM.h>
#include "Can.h"

uint8_t boardId = 1;                                  // Identifiant CAN de la carte

unsigned long time, stepTime = 2000;                  // Variables utilisées pour timer packet Test

void setup(){

  CAN.begin(CAN_80KBPS, MCP_16MHz);                   // Initialisation controleur CAN
  CAN.initCANMasksAndFilters();                       // Configuration des filtres pour n'accepter que id = 0
  CAN.setFilterId( 0, boardId );                      // Ajout aux filtre de l'id de la carte

}

void loop(){

  if( millis() > time + stepTime ){                   // Envoi d'un packet Test à intervalle régulier
    time += stepTime;

    canPacket pTest;
    pTest.id = boardId;
    pTest.msg[0] = millis() / 0x100;
    pTest.msg[1] = millis() % 0x100;
    CAN.sendPacket( pTest );

  }
  
  while( CAN.checkNewPacket() ){                      // Tant que packet CAN à lire
    canPacket p = CAN.getNewPacket();                    // Récupération du packet CAN
    CAN.sendPacket( p );                              // Renvoi du packet recu
  }

}
