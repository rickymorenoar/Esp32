#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP32Servo.h>

#define SS_PIN    5
#define RST_PIN   27
MFRC522 rfid(SS_PIN, RST_PIN);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

#define SERVO_PIN   26
#define SERVO_TUTUP 10
#define SERVO_BUKA  90
Servo door;

#define BUZZER_PIN 33
#define TRIG_PIN 14
#define ECHO_PIN 34
#define JARAK_TUTUP 15

String uidTerdaftar[] = {"FA69FB03", "04AB12CD"};
int jumlahUID = 2;

// Bunyi beep singkat
void beep(int freq, int dur){
  tone(BUZZER_PIN, freq);
  delay(dur);
  noTone(BUZZER_PIN);
}

// Suara alarm
void alarmPembobol(){
  for(int i = 0; i < 6; i++){
    tone(BUZZER_PIN, 2000); delay(200);
    tone(BUZZER_PIN, 1200); delay(200);
  }
  noTone(BUZZER_PIN);
}

// Update tulisan layar
void tampilOLED(String a, String b){
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(0, 25, a.c_str());
  oled.drawStr(0, 50, b.c_str());
  oled.sendBuffer();
}

// Cek ID kartu
bool cekAkses(String uid){
  for(int i = 0; i < jumlahUID; i++){
    if(uid == uidTerdaftar[i]) return true;
  }
  return false;
}

// Gerak servo halus
void servoKe(int dari, int ke){
  door.attach(SERVO_PIN, 500, 2400);
  int jarak = abs(ke - dari);
  int arah = (ke > dari) ? 1 : -1;

  for(int i = 0; i <= jarak; i++){
    float progress = (float)i / jarak;
    float easing = progress * progress;
    int posisi = dari + (jarak * easing * arah);
    door.write(posisi);
    delay(20);
  }
  delay(200);
  door.detach();
}

// Ukur jarak ultrasonic
long bacaJarak(){
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long durasi = pulseIn(ECHO_PIN, HIGH, 30000);
  if(durasi == 0) return 999;
  return durasi * 0.034 / 2;
}

void setup(){
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Wire.begin(21, 22);
  oled.begin();
  tampilOLED("SELAMAT", "DATANG");
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  rfid.PCD_Init();
  servoKe(SERVO_BUKA, SERVO_TUTUP);
}

void loop(){
  // Cek kartu rfid
  if(!rfid.PICC_IsNewCardPresent()) return;
  if(!rfid.PICC_ReadCardSerial()) return;

  // Ambil ID kartu
  String uid = "";
  for(byte i = 0; i < rfid.uid.size; i++){
    if(rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  if(cekAkses(uid)){
    tampilOLED("AKSES", "DITERIMA");
    beep(2000, 120);
    servoKe(SERVO_TUTUP, SERVO_BUKA);
    tampilOLED("SILAKAN", "MASUK");

    // Tunggu orang masuk (jarak dekat)
    while(true){
      if(bacaJarak() <= JARAK_TUTUP){
        beep(1500, 80);
        servoKe(SERVO_BUKA, SERVO_TUTUP);
        tampilOLED("SELAMAT", "DATANG");
        break;
      }
      delay(100);
    }
  } else {
    tampilOLED("ALARM!", "AKSES ILEGAL");
    alarmPembobol();
    delay(1000);
    tampilOLED("SELAMAT", "DATANG");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500);
}