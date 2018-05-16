#include <mcp_can.h>

#define SPI_CS_PIN 10
MCP_CAN CAN(SPI_CS_PIN);

// Déclaration structure pour une trame
struct canFrame{
  unsigned int id;
  unsigned char size;
  unsigned char msg[8];
};

void printFrame( canFrame f ){
  
    Serial.print( "ID: " );
    Serial.print( f.id );
    Serial.print( ", MSG: " );
    for( int i=0; i<8; i++ ){ 
      Serial.print( f.msg[i], HEX );
      Serial.print("\t");
    }
    Serial.println();
    
}

void setup(){
  
  Serial.begin(19200);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {
    Serial.println("pouet");
    delay(10);     // Initialisation bus CAN
  }
Serial.println("toto");
}

void loop(){

  // UART -> CAN
  if( Serial.available() ){
    char c = Serial.read();
    
    Serial.print("ToCAN :\t\t");

    canFrame f;
    f.id = 0;
    for( int i=0; i<8; i++ ) f.msg[i] = 0x00;

    if( c == 0x30 ){          // 0 -> position 1E80 -> 7808
        f.id = 1;
        f.msg[0] = 0xFF;
        f.msg[1] = 0x03;
        f.msg[2] = 0x00;
        f.msg[3] = 0x1E;
        f.msg[4] = 0x80;
        f.msg[5] = 0x00;
        f.msg[6] = 0x00;
        f.msg[7] = 0x00;
    } else if( c == 0x31 ){   // 1 -> position 7D00 -> 32000
        f.id = 1;
        f.msg[0] = 0xFF;
        f.msg[1] = 0x03;
        f.msg[2] = 0x00;
        f.msg[3] = 0x7D;
        f.msg[4] = 0x00;
        f.msg[5] = 0x00;
        f.msg[6] = 0x00;
        f.msg[7] = 0x00;
    } else if( c == 0x32 ){   // 2 -> trajectoire pos 7000 vmax 600 accel 200
        f.id = 1;
        f.msg[0] = 0xFF;
        f.msg[1] = 0x16;
        f.msg[2] = 0x00;
        f.msg[3] = 0x1E;
        f.msg[4] = 0x80;
        f.msg[5] = 0x02;
        f.msg[6] = 0x58;
        f.msg[7] = 0xC8;
    } else if( c == 0x33 ){   // 3 -> trajectoire pos 32000 vmax 600 accel 200
        f.id = 1;
        f.msg[0] = 0xFF;
        f.msg[1] = 0x16;
        f.msg[2] = 0x00;
        f.msg[3] = 0x7D;
        f.msg[4] = 0x00;
        f.msg[5] = 0x02;
        f.msg[6] = 0x58;
        f.msg[7] = 0xC8;
    } else if( c == 0x34 ){   // 3 -> trajectoire pos 32000 vmax 600 accel 200
        f.id = 1;
        f.msg[0] = 0xFF;
        f.msg[1] = 0x03;
        f.msg[2] = 0x00;
        f.msg[3] = 0x00;
        f.msg[4] = 0x00;
        f.msg[5] = 0x00;
        f.msg[6] = 0x00;
        f.msg[7] = 0x00;
    } else {
      Serial.print(" Nope ! \t");
    }

    CAN.sendMsgBuf( f.id, 0, f.size, f.msg );
    printFrame(f);

  }

   // CAN -> UART
  if( CAN_MSGAVAIL == CAN.checkReceive() ){                             // Test si des trames ont été recues

    canFrame f;
    CAN.readMsgBuf( &f.size, f.msg );
    f.id = CAN.getCanId();

    Serial.print("FromCAN :\t");
    printFrame(f);
    
  }

}
