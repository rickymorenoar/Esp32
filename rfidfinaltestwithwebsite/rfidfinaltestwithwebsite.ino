#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <time.h>

/* ================= LOGIN ADMIN ================= */
bool isLogin = false;
const char* ADMIN_USER = "admin";
const char* ADMIN_PASS = "1234";

/* ================= WIFI ================= */
const char* ssid = "Hpras199";
const char* password = "gohma8888";

/* ================= RFID ================= */
#define SS_PIN 5
#define RST_PIN 27
MFRC522 rfid(SS_PIN, RST_PIN);

/* ================= OLED ================= */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

/* ================= BUZZER ================= */
#define BUZZER_PIN 25
#define BUZZER_CH 0

/* ================= EEPROM ================= */
#define EEPROM_SIZE 4096
#define MAX_USER 20
#define MAX_LOG 80

struct User {
  char uid[12];
  char name[20];
};

struct LogAbsensi {
  char uid[12];
  char name[20];
  char waktu[24];
};

User users[MAX_USER];
LogAbsensi logs[MAX_LOG];
int userCount = 0;
int logCount = 0;

WebServer server(80);
String lastUID = "-";

/* ================= TIME ================= */
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;

/* ================= BUZZER ================= */
void beep(int f, int d){
  ledcWriteTone(BUZZER_CH, f);
  delay(d);
  ledcWriteTone(BUZZER_CH, 0);
}

/* ================= UTIL ================= */
int findUser(String uid){
  for(int i=0;i<userCount;i++){
    if(uid == users[i].uid) return i;
  }
  return -1;
}

String nowTime(){
  struct tm t;
  if(!getLocalTime(&t)) return "-";
  char buf[24];
  strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&t);
  return String(buf);
}

/* ================= EEPROM ================= */
void loadData(){
  EEPROM.get(0,userCount);
  EEPROM.get(4,users);
  EEPROM.get(600,logCount);
  EEPROM.get(604,logs);
}

void saveData(){
  EEPROM.put(0,userCount);
  EEPROM.put(4,users);
  EEPROM.put(600,logCount);
  EEPROM.put(604,logs);
  EEPROM.commit();
}

/* ================= OLED ================= */
void showOLED(String a,String b){
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(0,25,a.c_str());
  oled.drawStr(0,50,b.c_str());
  oled.sendBuffer();
}

/* ================= LOGIN PAGE ================= */
String loginPage(String msg=""){
  return R"rawliteral(
<!DOCTYPE html><html><head>
<meta name=viewport content=width=device-width>
<title>Login Admin</title>
<style>
body{
  background:#e5e7eb;
  font-family:Segoe UI,Arial;
  display:flex;
  justify-content:center;
  align-items:center;
  height:100vh
}
.card{
  background:white;
  padding:32px;
  width:340px;
  border-radius:14px;
  box-shadow:0 10px 30px rgba(0,0,0,.15)
}
h2{text-align:center;color:#0f172a}
input{
  width:100%;
  padding:12px;
  margin-top:12px;
  border-radius:8px;
  border:1px solid #ccc
}
button{
  width:100%;
  padding:12px;
  margin-top:16px;
  background:#0f172a;
  color:white;
  border:none;
  border-radius:8px;
  font-weight:600
}
.error{text-align:center;color:red;margin-top:10px}
</style></head><body>
<div class=card>
<h2>Login Admin</h2>
<form action=/login method=POST>
<input name=user placeholder=Username required>
<input name=pass type=password placeholder=Password required>
<button>LOGIN</button>
</form>
<div class=error>)rawliteral"+msg+R"rawliteral(</div>
</div></body></html>)rawliteral";
}

/* ================= MAIN PAGE ================= */
String mainPage(){
  String p = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name=viewport content=width=device-width>
<title>Absensi Kantor</title>
<style>
body{font-family:Segoe UI;background:#f1f5f9;margin:0}
header{
  background:#0f172a;
  color:white;
  padding:18px;
  font-size:22px;
  font-weight:600
}
.container{max-width:1100px;margin:auto;padding:24px}
.card{
  background:white;
  padding:22px;
  border-radius:14px;
  margin-bottom:24px;
  box-shadow:0 6px 20px rgba(0,0,0,.08)
}
h3{margin-top:0;color:#0f172a}
.uid{
  background:#020617;
  color:#38bdf8;
  padding:12px;
  border-radius:8px;
  font-family:monospace;
  margin:10px 0
}
input,select{
  width:100%;
  padding:12px;
  margin-top:10px;
  border-radius:8px;
  border:1px solid #ccc
}
button{
  padding:12px 18px;
  background:#2563eb;
  color:white;
  border:none;
  border-radius:8px;
  font-weight:600;
  margin-top:12px
}
table{width:100%;border-collapse:collapse;margin-top:12px}
th,td{
  padding:12px;
  border-bottom:1px solid #e5e7eb;
  text-align:left
}
th{background:#f8fafc}
.danger{background:#dc2626}
</style></head><body>

<header>🏢 Sistem Absensi Karyawan</header>
<div class=container>

<div class=card>
<h3>Tambah Karyawan</h3>
UID Terakhir
<div class=uid>)rawliteral"+lastUID+R"rawliteral(</div>
<form action=/add>
<input name=nama placeholder="Nama Karyawan" required>
<button>Simpan</button>
</form>
</div>

<div class=card>
<h3>Edit Nama Karyawan</h3>
<form action=/edit>
<select name=uid>)rawliteral";

  for(int i=0;i<userCount;i++){
    p+="<option value='"+String(users[i].uid)+"'>"+users[i].uid+" - "+users[i].name+"</option>";
  }

p+=R"rawliteral(
</select>
<input name=nama placeholder="Nama Baru" required>
<button>Update</button>
</form>
</div>

<div class=card>
<h3>Log Absensi</h3>
<table>
<tr><th>No</th><th>Nama</th><th>Waktu</th></tr>)rawliteral";

  for(int i=0;i<logCount;i++){
    p+="<tr><td>"+String(i+1)+"</td><td>"+logs[i].name+"</td><td>"+logs[i].waktu+"</td></tr>";
  }

p+=R"rawliteral(
</table>
<form action=/clear>
<button class=danger>Hapus Semua Log</button>
</form>
</div>

</div></body></html>)rawliteral";
  return p;
}

/* ================= SETUP ================= */
void setup(){
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  loadData();

  Wire.begin(21,22);
  oled.begin();
  showOLED("ABSENSI","SIAP");

  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  ledcSetup(BUZZER_CH,2000,8);
  ledcAttachPin(BUZZER_PIN,BUZZER_CH);

  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED) delay(500);

  configTime(gmtOffset_sec,0,ntpServer);

  server.on("/",[](){
    if(!isLogin) server.send(200,"text/html",loginPage());
    else server.send(200,"text/html",mainPage());
  });

  server.on("/login",[](){
    if(server.arg("user")==ADMIN_USER && server.arg("pass")==ADMIN_PASS){
      isLogin=true;
      beep(2000,100);beep(2500,100);
    }
    server.sendHeader("Location","/");
    server.send(303);
  });

  server.on("/add",[](){
    if(findUser(lastUID)==-1 && userCount<MAX_USER){
      lastUID.toCharArray(users[userCount].uid,12);
      server.arg("nama").toCharArray(users[userCount].name,20);
      userCount++; saveData();
    }
    server.sendHeader("Location","/");
    server.send(303);
  });

  server.on("/edit",[](){
    int i=findUser(server.arg("uid"));
    if(i!=-1){
      server.arg("nama").toCharArray(users[i].name,20);
      saveData();
    }
    server.sendHeader("Location","/");
    server.send(303);
  });

  server.on("/clear",[](){
    logCount=0; saveData();
    server.sendHeader("Location","/");
    server.send(303);
  });

  server.begin();
}

/* ================= LOOP ================= */
void loop(){
  server.handleClient();

  static unsigned long lastScan = 0;
  if (millis() - lastScan < 600) return;

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  lastScan = millis();

  lastUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) lastUID += "0";
    lastUID += String(rfid.uid.uidByte[i], HEX);
  }
  lastUID.toUpperCase();

  int idx = findUser(lastUID);
  if (idx != -1 && logCount < MAX_LOG) {
    strcpy(logs[logCount].name, users[idx].name);
    nowTime().toCharArray(logs[logCount].waktu, 24);
    logCount++;
    saveData();

    showOLED("HALO", users[idx].name);
    beep(2000,120);
  } else {
    showOLED("UID BARU", lastUID);
    beep(800,200);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
