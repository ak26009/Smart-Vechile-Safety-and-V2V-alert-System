#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

Adafruit_MMA8451 mma = Adafruit_MMA8451();

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial

  Wire.begin(21, 22); // SDA, SCL pins for ESP32

  // Use the correct I2C address 0x1C
  if (!mma.begin(0x1C)) {
    Serial.println("Could not find MMA8451 at 0x1C. Check wiring!");
    while (1);
  }

  mma.setRange(MMA8451_RANGE_2_G); // Options: 2G, 4G, 8G
  Serial.println("MMA8451 Found at 0x1C!");
}

void loop() {
  sensors_event_t event; 
  mma.getEvent(&event);

  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" | ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print(" | ");
  Serial.print("Z: "); Serial.println(event.acceleration.z);

  delay(500);
}
