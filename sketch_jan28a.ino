#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <U8g2lib.h>

/* ================= PIN OLED ================= */
#define SDA_PIN 8
#define SCL_PIN 9

/* ================= WIFI ================= */
const char* ssid = "HUD-GLASSES";
// const char* password = "12345678"; // MATIIN DULU

/* ================= SERVER ================= */
WebServer server(80);

/* ================= OLED ================= */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

/* ================= DATA ================= */
String hudText = "Waiting...";

/* ================= HANDLE WEB ================= */
void handleRoot() {
  if (server.hasArg("text")) {
    hudText = server.arg("text");
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  /* OLED */
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  /* RESET WIFI TOTAL */
  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_AP);
  delay(500);

  /* WIFI AP (OPEN) */
  bool apOK = WiFi.softAP(ssid);
  delay(2000);

  Serial.println(apOK ? "AP Oo" : "AP FAIL");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  /* WEB SERVER */
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 14, "HUD ONLINE");
  u8g2.drawStr(0, 36, hudText.c_str());
  u8g2.sendBuffer();
}
