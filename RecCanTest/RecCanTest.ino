#include <EEPROM.h>
#include "Can.h"

uint8_t boardId;                                      // Identifiant CAN de la carte

void setup(){

  boardId = EEPROM.read(0);

  CAN.begin(CAN_80KBPS, MCP_8MHz);                    // Initialisation controleur CAN
  CAN.initCANMasksAndFilters();                       // Configuration des filtres pour n'accepter que id = 0
  CAN.setFilterId( 0, boardId );                      // Ajout aux filtre de l'id de la carte

}

void loop(){
  
  while( CAN.checkNewPacket() ){                      // Tant que packet CAN à lire
    canPacket p = CAN.getNewPacket();                 // Récupération du packet CAN
    CAN.sendPacket( p );                              // Renvoi du packet recu
  }

}
