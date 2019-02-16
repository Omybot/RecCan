#ifndef CAN_H
#define CAN_H

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

enum canFunction{
	PositionAsk = 0x01,
	PositionResponse = 0x02,
	PositionSet = 0x03,
	PositionMinAsk = 0x04,
	PositionMinResponse = 0x05,
	PositionMinSet = 0x06,
	PositionMaxAsk = 0x07,
	PositionMaxResponse = 0x08,
	PositionMaxSet = 0x09,
	SpeedAsk = 0x0A,
	SpeedResponse = 0x0B,
	SpeedSet = 0x0C,
	TorqueMaxAsk = 0x0D,
	TorqueMaxResponse = 0x0E,
	TorqueMaxSet = 0x0F,
	TorqueCurrentAsk = 0x10,
	TorqueCurrentResponse = 0x11,
	AccelerationAsk = 0x12,
	AccelerationResponse = 0x13,
	AccelerationSet = 0x14,
	TargetSet = 0x15,
	TrajectorySet = 0x16,

	SetScore = 0xA0,

	Debug = 0xF0,
	DebugAsk = 0xF1,
	DebugResponse = 0xF2,
};

#define SPI_CAN_CS_PIN	10	// Pin du chip select du composant MCP2515

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

