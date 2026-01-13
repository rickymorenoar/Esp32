#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <time.h>

/* ===== WIFI ===== */
const char* ssid = "Hpras199";
const char* password = "gohma8888";

/* ===== RFID ===== */
#define SS_PIN 5
#define RST_PIN 27
MFRC522 rfid(SS_PIN, RST_PIN);

/* ===== OLED ===== */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

/* ===== BUZZER ===== */
#define BUZZER_PIN 25
#define BUZZER_CH 0

/* ===== EEPROM ===== */
#define EEPROM_SIZE 4096
#define MAX_USER 20
#define MAX_LOG 30

#define ADDR_USERCOUNT 0
#define ADDR_USERS     4
#define ADDR_LOGCOUNT  800
#define ADDR_LOGS      804

struct User {
  char uid[12];
  char name[20];
};

struct LogAbsensi {
  char name[20];
  char waktu[24];
};

User users[MAX_USER];
LogAbsensi logs[MAX_LOG];
int userCount = 0;
int logCount = 0;

/* ===== SERVER ===== */
WebServer server(80);
String lastUID = "-";

/* ===== TIME ===== */
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;

/* ===== OLED IDLE ===== */
unsigned long lastScanMillis = 0;
const unsigned long IDLE_TIMEOUT = 10000;
bool oledIdle = true;

/* ===== BUZZER ===== */
void beep() {
  ledcWriteTone(BUZZER_CH, 2000);
  delay(120);
  ledcWriteTone(BUZZER_CH, 0);
}

/* ===== OLED ===== */
void showOLED(String a, String b) {
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(0, 22, a.c_str());
  oled.drawStr(0, 45, b.c_str());
  oled.sendBuffer();
}

/* ===== EEPROM ===== */
void loadData() {
  EEPROM.get(ADDR_USERCOUNT, userCount);
  EEPROM.get(ADDR_USERS, users);
  EEPROM.get(ADDR_LOGCOUNT, logCount);
  EEPROM.get(ADDR_LOGS, logs);

  if (userCount < 0 || userCount > MAX_USER) userCount = 0;
  if (logCount < 0 || logCount > MAX_LOG) logCount = 0;
}

void saveData() {
  EEPROM.put(ADDR_USERCOUNT, userCount);
  EEPROM.put(ADDR_USERS, users);
  EEPROM.put(ADDR_LOGCOUNT, logCount);
  EEPROM.put(ADDR_LOGS, logs);
  EEPROM.commit();
}

/* ===== UTIL ===== */
int findUser(String uid) {
  for (int i = 0; i < userCount; i++) {
    if (uid == users[i].uid) return i;
  }
  return -1;
}

String getTimeStr() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "-";
  char buf[24];
  strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", &timeinfo);
  return String(buf);
}

/* ===== WEB ===== */
String webpage() {
  int idx = findUser(lastUID);

  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Sistem Absensi Kantor</title>
<style>
body{
  margin:0;
  font-family: Arial, Helvetica, sans-serif;
  background:#f4f6f9;
}
.header{
  background:#1e293b;
  color:white;
  padding:15px 30px;
  font-size:22px;
  font-weight:bold;
}
.container{
  padding:30px;
}
.card{
  background:white;
  padding:20px;
  border-radius:10px;
  box-shadow:0 4px 10px rgba(0,0,0,0.08);
  margin-bottom:25px;
}
.label{
  font-weight:bold;
  margin-bottom:5px;
}
.uid-box{
  font-size:18px;
  color:#2563eb;
  margin-bottom:15px;
}
input{
  width:100%;
  padding:10px;
  border-radius:6px;
  border:1px solid #ccc;
  margin-bottom:10px;
}
button{
  background:#2563eb;
  color:white;
  border:none;
  padding:10px 18px;
  border-radius:6px;
  cursor:pointer;
}
button:hover{
  background:#1e40af;
}
table{
  width:100%;
  border-collapse:collapse;
  margin-top:10px;
}
th, td{
  padding:10px;
  border-bottom:1px solid #e5e7eb;
  text-align:left;
}
th{
  background:#e5e7eb;
}
.footer{
  text-align:center;
  color:#6b7280;
  margin-top:30px;
  font-size:13px;
}
</style>
</head>
<body>

<div class="header">SISTEM ABSENSI KANTOR</div>

<div class="container">

  <div class="card">
    <div class="label">UID Terakhir</div>
    <div class="uid-box">)rawliteral";

  page += lastUID;

  page += R"rawliteral(</div>

    <form action="/save">
      <div class="label">Nama Karyawan</div>
      <input name="nama" placeholder="Masukkan nama karyawan" value=")rawliteral";

  if (idx != -1) page += users[idx].name;

  page += R"rawliteral(">
      <button>Simpan / Update</button>
    </form>
  </div>

  <div class="card">
    <div class="label">Log Absensi</div>
    <table>
      <tr>
        <th>No</th>
        <th>Nama</th>
        <th>Waktu</th>
      </tr>
)rawliteral";

  for (int i = 0; i < logCount; i++) {
    page += "<tr><td>" + String(i+1) + "</td><td>";
    page += logs[i].name;
    page += "</td><td>";
    page += logs[i].waktu;
    page += "</td></tr>";
  }

  page += R"rawliteral(
    </table>
  </div>

  <div class="footer">
    © Sistem Absensi ESP32 • RFID • OLED
  </div>

</div>
</body>
</html>
)rawliteral";

  return page;
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  EEPROM.begin(EEPROM_SIZE);
  loadData();

  ledcSetup(BUZZER_CH, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);

  Wire.begin(21,22);
  oled.begin();
  showOLED("ABSENSI", "SIAP");

  SPI.begin();            // 🔥 FIX RFID
  rfid.PCD_Init();

  Serial.println("CONNECT WIFI...");
  WiFi.begin(ssid, password);

  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - t > 20000) break;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWIFI CONNECTED");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWIFI GAGAL");
  }

  configTime(gmtOffset_sec, 0, ntpServer);

  server.on("/", [](){ server.send(200,"text/html",webpage()); });

  server.on("/save", [](){
    if (lastUID == "-" || server.arg("nama") == "") {
      server.send(400,"text/plain","Gagal");
      return;
    }

    String nama = server.arg("nama");
    int idx = findUser(lastUID);

    if (idx != -1) {
      nama.toCharArray(users[idx].name, 20);
    } else if (userCount < MAX_USER) {
      lastUID.toCharArray(users[userCount].uid, 12);
      nama.toCharArray(users[userCount].name, 20);
      userCount++;
    }

    saveData();
    server.sendHeader("Location","/");
    server.send(303);
  });

  server.begin();
}

/* ===== LOOP ===== */
void loop() {
  server.handleClient();

  if (!oledIdle && millis() - lastScanMillis > IDLE_TIMEOUT) {
    showOLED("ABSENSI", "SIAP");
    oledIdle = true;
    lastUID = "-";
  }

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  lastScanMillis = millis();
  oledIdle = false;

  lastUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) lastUID += "0";
    lastUID += String(rfid.uid.uidByte[i], HEX);
  }
  lastUID.toUpperCase();

  Serial.println("SCAN: " + lastUID);
  beep();

  int idx = findUser(lastUID);
  if (idx != -1 && logCount < MAX_LOG) {
    String waktu = getTimeStr();
    strcpy(logs[logCount].name, users[idx].name);
    waktu.toCharArray(logs[logCount].waktu, 24);
    logCount++;
    saveData();
    showOLED("HALO", users[idx].name);
  } else {
    showOLED("UID BARU", lastUID);
  }

  rfid.PICC_HaltA();
  delay(800);
}
