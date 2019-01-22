#include "Can.h"

/**
 * \brief Constructeur de la classe
 */
CANClass::CANClass(){

	mcpcan = MCP_CAN(SPI_CAN_CS_PIN);
	
}

/**
 * \brief Destructeur de la classe
 */
CANClass::~CANClass(){

}

/**
 * \fn void begin( uint8_t speed )
 * \brief Fonction (bloquante) d'initialisation du bus CAN avec un oscillateur de 16MHz et la vitesse de communication désirée
 *
 * \param speed : (uint8_t) vitesse de communication sur le bus en baud (i.e. : CAN_80KBPS)
 * \return code retourné par la fonction begin de la librairie MCP_CAN
 */
void CANClass::begin( uint8_t speed, uint8_t clock ){
	while( mcpcan.begin( speed, clock ) ) delay(10);
}

/**
 * \fn void sendPacket( canPacket p )
 * \brief Fonction qui envoi un packet sur le bus CAN
 *
 * tps d'exec < 112 µs (Arduino Uno)
 */
uint8_t CANClass::sendPacket( canPacket p ){
	return mcpcan.trySendMsgBuf( p.id, 0, 0, 8, p.msg );
}

/**
 * \fn bool checkNewPacket()
 * \brief Fonction qui vérifie si un nouveau packet a été recu
 *
 * tps d'exec < 24 µs (Arduino Uno)
 *
 * \return true si packet recu
 */
bool CANClass::checkNewPacket(){
	if(CAN_MSGAVAIL == mcpcan.checkReceive()) return true;
	else return false;
}

/**
 * \fn canPacket getNewPacket()
 * \brief Fonction qui retourne le nouveau packet récupéré
 *
 * tps d'exec < 92 µs (Arduino Uno)
 *
 * \param pBuf : packet récupéré
 */
canPacket CANClass::getNewPacket(){

	canPacket pBuf;						// Création du packet

	mcpcan.readMsgBuf(8, pBuf.msg);
	pBuf.id = mcpcan.getCanId();

	return pBuf;

}

/**
 * \fn void initCANMasksAndFilters()
 * \brief Initialisation des masques et filtres pour le bus CAN
 */
void CANClass::initCANMasksAndFilters(){

	// Buffer de récéption RXB0 (1 masque et 2 filtres associés)
	mcpcan.init_Mask( 0, 0, 0xFFFF );			// Par défaut, le masque 1 bloque tout type d'identifiant, seul les filtres peuvent autoriser les identifiants
	mcpcan.init_Filt( 0, 0, 0 );				// Filtre qui n'autorique que l'id 0
	mcpcan.init_Filt( 1, 0, 0 );				// Filtre qui n'autorique que l'id 0
	
	// Buffer de récéption RXB1 (1 masque et 4 filtres associés)
	mcpcan.init_Mask( 1, 0, 0xFFFF );			// Par défaut, le masque 1 bloque tout type d'identifiant, seul les filtres peuvent autoriser les identifiants
	mcpcan.init_Filt( 2, 0, 0 );				// Filtre qui n'autorique que l'id 0
	mcpcan.init_Filt( 3, 0, 0 );				// Filtre qui n'autorique que l'id 0
	mcpcan.init_Filt( 4, 0, 0 );				// Filtre qui n'autorique que l'id 0
	mcpcan.init_Filt( 5, 0, 0 );				// Filtre qui n'autorique que l'id 0

}

/**
 * \fn void setFilterId( filterNum, filterId )
 * \brief Mise à jour de la valeur du filtre désiré
 *
 * \param filterNum : (uint8_t) Numéro du filtre à mettre a jour (0 à 5)
 * \param filterId : (uint16_t) valeur du filtre (0 à 0x7FF)
 */
void CANClass::setFilterId( uint8_t filterNum, uint16_t filterId ){
	mcpcan.init_Filt( filterNum, 0, filterId );
}

CANClass CAN;							// Auto-Instantiation d'un objet CANClass

