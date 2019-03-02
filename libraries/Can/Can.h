#ifndef CAN_H
#define CAN_H

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

enum CanFunction{
	Debug = 0xF0,
	DebugAsk = 0xF1,
	DebugResponse = 0xF2,
};

#define SPI_CAN_CS_PIN	8	// Pin du chip select du composant MCP2515

#define	CAN_PACKET_SIZE	10	// Taille de chaque packet en octet

#define MCP_16MHz      1
#define MCP_8MHz       2

#ifndef canPacket_struct
#define canPacket_struct
struct canPacket{
	int id = 0;
	uint8_t msg[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
};
#endif

/**
 * /class	CANClass
 * /brief	Classe CANClass
 */
class CANClass {

	public :
		CANClass();
		~CANClass();

		void begin( uint8_t speed = CAN_80KBPS, uint8_t clock = MCP_16MHz );

		bool checkNewPacket();
		canPacket getNewPacket();

		uint8_t sendPacket( canPacket p );

		void initCANMasksAndFilters();
		void setFilterId( uint8_t filterNum, uint16_t filterId );

	private :

		MCP_CAN mcpcan;

};

extern CANClass CAN;

#endif

