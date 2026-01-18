#include <WiFi.h>
#include <WebServer.h>
#include <DHT11.h>

// ============ PIN ============
#define DHT_PIN   4
#define GAS_PIN   34
#define BUZZER    26

// ============ LIMIT ==========
#define GAS_LIMIT   2500
#define TEMP_LIMIT  40

// ============ WIFI AP ========
const char* ssid = "ESP32-SAFETY";
const char* password = "12345678";

WebServer server(80);
DHT11 dht(DHT_PIN);

bool buzzerEnable = true;

// ===== BUZZER LOW TRIGGER =====
void buzzerON()  { digitalWrite(BUZZER, LOW); }
void buzzerOFF() { digitalWrite(BUZZER, HIGH); }

// ===== POLA ALARM =====
void gasAlarm() {
  buzzerON(); delay(150);
  buzzerOFF(); delay(150);
}

void tempAlarm() {
  buzzerON();
  delay(1000);
  buzzerOFF();
}

// ===== DATA JSON (REAL TIME) =====
void handleData() {
  int temp = dht.readTemperature();
  int gas  = analogRead(GAS_PIN);

  String status = "AMAN";
  if (gas > GAS_LIMIT) status = "BAHAYA GAS";
  else if (temp > TEMP_LIMIT) status = "SUHU PANAS";

  String json = "{";
  json += "\"temp\":" + String(temp) + ",";
  json += "\"gas\":" + String(gas) + ",";
  json += "\"status\":\"" + status + "\",";
  json += "\"buzzer\":\"" + String(buzzerEnable ? "ON" : "OFF") + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// ===== WEB PAGE =====
void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{font-family:Arial;text-align:center;background:#111;color:white;}
.box{padding:20px;border-radius:15px;margin:10px;background:#222;}
.status{font-size:24px;font-weight:bold;}
button{padding:15px 25px;font-size:16px;margin:5px;border-radius:10px;}
</style>
</head>
<body>

<h2>SMART HOME SAFETY</h2>

<div class="box">
  <p>Suhu: <b><span id="temp">--</span> °C</b></p>
  <p>Gas: <b><span id="gas">--</span></b></p>
  <p class="status" id="status">---</p>
</div>

<div class="box">
  <p>Buzzer: <b><span id="buzzer">---</span></b></p>
  <a href="/on"><button>BUZZER ON</button></a>
  <a href="/off"><button>BUZZER OFF</button></a>
</div>

<script>
setInterval(() => {
  fetch("/data")
    .then(res => res.json())
    .then(d => {
      document.getElementById("temp").innerText = d.temp;
      document.getElementById("gas").innerText = d.gas;
      document.getElementById("status").innerText = d.status;
      document.getElementById("buzzer").innerText = d.buzzer;

      let s = document.getElementById("status");
      if (d.status.includes("GAS")) s.style.color = "red";
      else if (d.status.includes("SUHU")) s.style.color = "orange";
      else s.style.color = "lime";
    });
}, 1000);
</script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void handleOn() {
  buzzerEnable = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  buzzerEnable = false;
  buzzerOFF();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  buzzerOFF();

  Serial.println("Warming gas sensor...");
  delay(60000);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
}

void loop() {
  server.handleClient();

  int temp = dht.readTemperature();
  int gas  = analogRead(GAS_PIN);

  if (buzzerEnable) {
    if (gas > GAS_LIMIT) {
      gasAlarm();
    } else if (temp > TEMP_LIMIT) {
      tempAlarm();
    } else {
      buzzerOFF();
    }
  }

  delay(500);
}
