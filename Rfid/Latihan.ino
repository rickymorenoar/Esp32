#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 5
#define RST_PIN 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

MFRC522 rfid(SDA_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 🔧 GANTI INI
const char* ssid = "Id_Wifi_You";
const char* password = "Pasword_Wifi_You";
String server = "http://Your_IP_Computer/admin/simpan.php?uid=";

void setup() {
  Serial.begin(115200);

  // OLED
  Wire.begin(21,22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Tes RFID");
  display.display();

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // WiFi
  WiFi.begin(ssid, password);
  display.println("WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  display.println("WiFi OK");
  display.display();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.println(uid);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("UID:");
  display.println(uid);
  display.display();

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server + uid);
    int code = http.GET();
    http.end();

    display.println(code == 200 ? "Terkirim" : "Gagal");
    display.display();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}
