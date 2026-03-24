#include <SPI.h>
#include <LoRa.h>

// ---- DO NOT CHANGE PINS ----
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

    String data = "";

    while (LoRa.available()) {
      data += (char)LoRa.read();
    }

    // Example expected string:
    // T:28.8,N:1567,F:102

    float temp = 0;
    int noise = 0;
    int flame = 0;

    int tIndex = data.indexOf("T:");
    int nIndex = data.indexOf("N:");
    int fIndex = data.indexOf("F:");

    if (tIndex != -1 && nIndex != -1 && fIndex != -1) {
      temp  = data.substring(tIndex + 2, nIndex - 1).toFloat();
      noise = data.substring(nIndex + 2, fIndex - 1).toInt();
      flame = data.substring(fIndex + 2).toInt();
    }

    Serial.println("========== LORA DATA ==========");
    Serial.print("Temp  : ");
    Serial.print(temp, 2);
    Serial.println(" °C");

    Serial.print("Noise : ");
    Serial.println(noise);

    Serial.print("Flame : ");
    Serial.println(flame);

    Serial.print("RSSI  : ");
    Serial.println(LoRa.packetRssi());
    Serial.println("===============================");
  }
}
