#include <SPI.h>
#include <LoRa.h>

// Pin definitions (from your table)
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2

void setup() {
  Serial.begin(9600);
  while (!Serial);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }

  Serial.println("LoRa Receiver Started");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {

    Serial.print("Received: ");

    while (LoRa.available()) {
      String data = LoRa.readString();
      Serial.print(data);
    }

    Serial.print(" | RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}