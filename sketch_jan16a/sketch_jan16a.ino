#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP32Servo.h>

/* ================= RFID ================= */
#define SS_PIN   5
#define RST_PIN  27
MFRC522 rfid(SS_PIN, RST_PIN);

/* ================= OLED ================= */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

/* ================= SERVO ================= */
#define SERVO_PIN   26
#define SERVO_TUTUP 25     // DIKECILIN
#define SERVO_BUKA  90     // AMAN UNTUK SG90
Servo door;

/* ================= BUZZER PASIF ================= */
#define BUZZER_PIN 25

/* ================= UID TERDAFTAR ================= */
String uidTerdaftar[] = {
  "FA69FB03",
  "04AB12CD"
};
int jumlahUID = 2;

/* ================= UTIL ================= */
void beep(int freq, int dur){
  tone(BUZZER_PIN, freq);
  delay(dur);
  noTone(BUZZER_PIN);
}

void tampilOLED(String a, String b){
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(0, 25, a.c_str());
  oled.drawStr(0, 50, b.c_str());
  oled.sendBuffer();
}

bool cekAkses(String uid){
  for(int i = 0; i < jumlahUID; i++){
    if(uid == uidTerdaftar[i]) return true;
  }
  return false;
}

/* ================= GERAK SERVO HALUS ================= */
void servoKe(int dari, int ke){
  door.attach(SERVO_PIN, 500, 2400);
  if(dari < ke){
    for(int p = dari; p <= ke; p++){
      door.write(p);
      delay(12);
    }
  } else {
    for(int p = dari; p >= ke; p--){
      door.write(p);
      delay(12);
    }
  }
  delay(300);
  door.detach();   // 🔥 KUNCI UTAMA
}

/* ================= SETUP ================= */
void setup(){
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(21, 22);
  oled.begin();
  tampilOLED("SELAMAT", "DATANG");

  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();

  servoKe(SERVO_BUKA, SERVO_TUTUP); // pastikan terkunci
}

/* ================= LOOP ================= */
void loop(){

  if(!rfid.PICC_IsNewCardPresent()) return;
  if(!rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for(byte i = 0; i < rfid.uid.size; i++){
    if(rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println(uid);

  if(cekAkses(uid)){
    tampilOLED("AKSES", "DITERIMA");
    beep(2000, 120);

    // ===== BUKA =====
    servoKe(SERVO_TUTUP, SERVO_BUKA);

    delay(3000); // pintu terbuka

    beep(1500, 80);

    // ===== TUTUP =====
    servoKe(SERVO_BUKA, SERVO_TUTUP);

    tampilOLED("SELAMAT", "DATANG");

  } else {
    tampilOLED("AKSES", "DITOLAK");
    beep(800, 400);
    delay(1500);
    tampilOLED("SELAMAT", "DATANG");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  while(rfid.PICC_IsNewCardPresent()){
    delay(50);
  }
  delay(300);
}
