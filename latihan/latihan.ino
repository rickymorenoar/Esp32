#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

const int mqPin = 34;  // Pin ADC valid di ESP32
const int buzzerPin = 25;

const int gasThreshold = 300;  // Sesuaikan threshold

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  delay(300000); // Pemanasan 5 menit
}

void loop() {
  int sensorValue = analogRead(mqPin);

  Serial.print("MQ135 Value: ");
  Serial.println(sensorValue);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 15, "Sensor MQ135");
  char buf[20];
  sprintf(buf, "Value: %d", sensorValue);
  u8g2.drawStr(0, 35, buf);

  if(sensorValue > gasThreshold){
    u8g2.drawStr(0, 55, "WARNING! GAS DETECTED");
    tone(buzzerPin, 1000);
  } else {
    noTone(buzzerPin);
  }

  u8g2.sendBuffer();
  delay(500);
}
