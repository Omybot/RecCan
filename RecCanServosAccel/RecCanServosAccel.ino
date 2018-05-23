#include <MyServo.h>
#include <Utils.h>
#include <EEPROM.h>
#include <MyQueue.h>
#include <SPI.h>
#include "mcp_can.h"
#include <MyServoHandler.h>

/////////////////////////////////
// Gestion bus CAN
/////////////////////////////////

#define SPI_CS_PIN 8
MCP_CAN CAN(SPI_CS_PIN);

// Déclaration structure pour une trame
struct canFrame {
	unsigned int id;
	byte size;
	byte msg[8];
};

/////////////////////////////////
// Gestion des servos
/////////////////////////////////

MyServoHandler servos;

/////////////////////////////////
// Gestion des interruptions
/////////////////////////////////
ISR(TIMER1_COMPA_vect) {
	servos.timer1Interrupt();
}
ISR(TIMER2_COMPA_vect) {
	servos.timer2Interrupt();
}

/////////////////////////////////
// SETUP
/////////////////////////////////
void setup() {

	//Serial.begin(115200);

	int canId = EEPROM.read(0);

	// Gestion des pins
	servos.attach();

	// Initialisation CAN
	while (CAN_OK != CAN.begin(CAN_500KBPS)) { delay(10); }

	// Filtre pour ne prendre en compte que les trames désirées
	CAN.init_Mask(0, 0, 0x7ff);
	CAN.init_Mask(1, 0, 0x7ff);
	CAN.init_Filt(0, 0, canId);    // Ne prend en compte que les trames d'identifiant CAN_ID

}

/////////////////////////////////
// LOOP
/////////////////////////////////
void loop() {

	if (CAN_MSGAVAIL == CAN.checkReceive()) {                             // Test si des trames ont été recues

		canFrame received;
		CAN.readMsgBuf(&received.size, received.msg);
		received.id = CAN.getCanId();

		if (received.id == 0) return;

		byte command = received.msg[0];
		byte servoId = received.msg[1];

		canFrame response;
		response.id = received.id;
		for (int i = 0; i < 7; i++) response.msg[i] = received.msg[i];
		response.size = 8;

		switch (command) {

			case ServoFunction::PositionAsk:{
				unsigned int position = servos.getPosition(servoId);
				response.msg[0] = ServoFunction::PositionResponse;
				response.msg[2] = Utils::GetMSB(position);
				response.msg[3] = Utils::GetLSB(position);
				break;
			}

			case ServoFunction::PositionSet:{
				unsigned int newPosition = Utils::ReadInt16(received.msg, 2);
				servos.setPosition(servoId, newPosition);
				break;
			}

			case ServoFunction::SpeedAsk:{
				unsigned int speed = servos.getSpeed(servoId);
				response.msg[0] = ServoFunction::SpeedResponse;
				response.msg[2] = Utils::GetMSB(speed);
				response.msg[3] = Utils::GetLSB(speed);
				break;
			}

			case ServoFunction::SpeedSet:{
				unsigned int newSpeed = Utils::ReadInt16(received.msg, 2);
				servos.setMaxSpeed(servoId, newSpeed);
				break;
			}

			/*case ServoFunction::TorqueCurrentAsk:{
				unsigned int torque = servos.getTorque(servoId);
				response.msg[0] = ServoFunction::TorqueCurrentResponse;
				response.msg[2] = Utils::GetMSB(torque);
				response.msg[3] = Utils::GetLSB(torque);
				break;
			}

			case ServoFunction::TorqueMaxSet:{
				unsigned int newTorque = Utils::ReadInt16(received.msg, 2);
				servos.setMaxTorque(servoId, newTorque);
				break;
			}

			case ServoFunction::TorqueMaxAsk:{
				unsigned int maxTorque = servos.getMaxTorque(servoId);
				response.msg[0] = ServoFunction::TorqueMaxResponse;
				response.msg[2] = Utils::GetMSB(maxTorque);
				response.msg[3] = Utils::GetLSB(maxTorque);
				break;
			}*/

			case ServoFunction::AccelAsk:{
				unsigned int accel = servos.getAccel(servoId);
				response.msg[1] = ServoFunction::AccelResponse;
				response.msg[3] = Utils::GetMSB(accel);
				response.msg[4] = Utils::GetLSB(accel);
				break;
			}

			case ServoFunction::AccelSet:{
				unsigned int newAccel = Utils::ReadInt16(received.msg, 2);
				servos.setAccel(servoId, newAccel);
				break;
			}

			case ServoFunction::TargetSet:{
				unsigned int target = Utils::ReadInt16(received.msg, 2);
				servos.setTargetPosition(servoId, target);
				break;
			}

			case ServoFunction::TrajectorySet:{
				unsigned int trajTarget = Utils::ReadInt16(received.msg, 2);
				unsigned int trajVmax = Utils::ReadInt16(received.msg, 4);
				unsigned int trajAccel = Utils::ReadInt16(received.msg, 6);
				servos.setTrajectory(servoId, trajTarget, trajVmax, trajAccel);
				break;
			}
		}

		// Retour réponse
		CAN.sendMsgBuf(response.id, 0, response.size, response.msg);               // Envoi du message sur le bus CAN

	}
}


