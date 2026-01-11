#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <U8g2lib.h>

// ================= WIFI =================
const char* ssid = "Hprass199";
const char* pass = "gohma8888";

// ================= PIN =================
#define SS_PIN     5
#define RST_PIN    27
#define BUZZER_PIN 25
#define BUZZER_CH  0

// ================= OBJECT =================
WebServer server(80);
MFRC522 rfid(SS_PIN, RST_PIN);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

String lastUID = "";
String uidDB[20];
int uidCount = 0;

unsigned long lastScan = 0;

// ================= BUZZER =================
void beep() {
  ledcWriteTone(BUZZER_CH, 2000);
  delay(80);
  ledcWriteTone(BUZZER_CH, 0);
}

// ================= WEB PAGE =================
String webpage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Absensi Kantor</title>
<style>
body{font-family:Arial;background:#f1f5f9}
.card{width:420px;margin:60px auto;background:#fff;
padding:20px;border-radius:10px;box-shadow:0 10px 20px #bbb}
input,button{width:100%;padding:10px;margin-top:10px}
button{background:#2563eb;color:#fff;border:none;border-radius:5px}
h2{text-align:center}
</style>
</head>

<body>

<div class="card" id="login">
<h2>Login Admin</h2>
<input id="u" placeholder="Username">
<input id="p" type="password" placeholder="Password">
<button onclick="login()">Login</button>
</div>

<div class="card" id="dash" style="display:none">
<h2>Dashboard Absensi</h2>
<p><b>UID Terakhir:</b></p>
<h3 id="uid">-</h3>
<button onclick="add()">Daftarkan UID</button>
</div>

<script>
function login(){
  if(u.value=="admin" && p.value=="1234"){
    login.style.display="none";
    dash.style.display="block";
  } else alert("Login salah");
}

setInterval(()=>{
  fetch('/uid').then(r=>r.text()).then(t=>{
    if(t!="") uid.innerHTML=t;
  });
},1000);

function add(){
  fetch('/add').then(r=>r.text()).then(t=>{
    alert(t);
  });
}
</script>

</body>
</html>
)rawliteral";
}

// ================= SERVER =================
void setupServer() {
  server.on("/", []() {
    server.send(200, "text/html", webpage());
  });

  server.on("/uid", []() {
    server.send(200, "text/plain", lastUID);
  });

  server.on("/add", []() {
    if (lastUID == "") {
      server.send(400, "text/plain", "Tidak ada UID");
      return;
    }
    for (int i = 0; i < uidCount; i++) {
      if (uidDB[i] == lastUID) {
        server.send(200, "text/plain", "UID sudah terdaftar");
        return;
      }
    }
    if (uidCount < 20) {
      uidDB[uidCount++] = lastUID;
      server.send(200, "text/plain", "UID berhasil disimpan");
    } else {
      server.send(500, "text/plain", "Database penuh");
    }
  });

  server.begin();
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  oled.begin();
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(10, 30, "Tempel Kartu");
  oled.sendBuffer();

  ledcSetup(BUZZER_CH, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  setupServer();
}

// ================= LOOP =================
void loop() {
  server.handleClient();

  if (millis() - lastScan < 1500) return;
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  lastScan = millis();

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++)
    uid += String(rfid.uid.uidByte[i], HEX);
  uid.toUpperCase();

  lastUID = uid;

  Serial.println("Scan UID: " + uid);

  beep();

  oled.clearBuffer();
  oled.drawStr(0, 20, "UID:");
  oled.drawStr(0, 40, uid.c_str());
  oled.sendBuffer();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
