#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS   5     // MCP2515 CS pin
#define CAN_INT  4     // MCP2515 INT pin (optional but recommended)

MCP_CAN CAN0(CAN_CS);

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin();  // ESP32 default: SCK=18, MISO=19, MOSI=23

  Serial.println("Initializing MCP2515 CAN...");

  // Initialize MCP2515 (adjust clock if needed: MCP_8MHZ / MCP_16MHZ)
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Init OK");
  } else {
    Serial.println("CAN Init Failed");
    while (1);
  }

  CAN0.setMode(MCP_NORMAL);   // Normal mode (not loopback)
  pinMode(CAN_INT, INPUT);

  Serial.println("CAN Ready...");
}

void loop() {
  unsigned long rxId;
  unsigned char len;
  unsigned char rxBuf[8];

  if (CAN0.checkReceive() == CAN_MSGAVAIL) {

    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    if (rxId == 0x300 && len >= 6) {

      uint16_t temp_raw = ((uint16_t)rxBuf[0] << 8) | rxBuf[1];
      uint16_t noise    = ((uint16_t)rxBuf[2] << 8) | rxBuf[3];
      uint16_t flame    = ((uint16_t)rxBuf[4] << 8) | rxBuf[5];

      float temp = temp_raw / 10.0f;

      Serial.print("CAN ID: 0x");
      Serial.print(rxId, HEX);
      Serial.print("  Temp: ");
      Serial.print(temp, 2);
      Serial.print("°C  Noise: ");
      Serial.print(noise);
      Serial.print("  Flame: ");
      Serial.println(flame);
    }
  }
}

