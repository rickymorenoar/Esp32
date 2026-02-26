#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP32Servo.h>

/* ===== RFID ===== */
#define SS_PIN 5
#define RST_PIN 27
MFRC522 rfid(SS_PIN, RST_PIN);

/* ===== OLED ===== */
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);

/* ===== SERVO ===== */
#define SERVO_PIN 26
Servo door;

/* ===== BUZZER ===== */
#define BUZZER_PIN 25
#define BUZZER_CH 0

/* ===== AKSES UID (GANTI PUNYA KAMU) ===== */
String accessUID[] = {
  "FA69FB03",
  "04AB12CD"
};
int totalUID = 2;

/* ===== UTIL ===== */
void beep(int f, int d){
  ledcWriteTone(BUZZER_CH, f);
  delay(d);
  ledcWriteTone(BUZZER_CH, 0);
}

bool isAllowed(String uid){
  for(int i=0;i<totalUID;i++){
    if(uid == accessUID[i]) return true;
  }
  return false;
}

void showOLED(String a,String b){
  oled.clearBuffer();
  oled.setFont(u8g2_font_ncenB08_tr);
  oled.drawStr(0,25,a.c_str());
  oled.drawStr(0,50,b.c_str());
  oled.sendBuffer();
}

/* ===== SETUP ===== */
void setup(){
  Serial.begin(115200);

  Wire.begin(21,22);
  oled.begin();
  showOLED("PINTU HOTEL","SIAP");

  SPI.begin(18,19,23,5);
  rfid.PCD_Init();

  door.attach(SERVO_PIN);
  door.write(0); // pintu terkunci

  ledcSetup(BUZZER_CH,2000,8);
  ledcAttachPin(BUZZER_PIN,BUZZER_CH);
}

/* ===== LOOP ===== */
void loop(){
  if(!rfid.PICC_IsNewCardPresent()) return;
  if(!rfid.PICC_ReadCardSerial()) return;

  String uid="";
  for(byte i=0;i<rfid.uid.size;i++){
    if(rfid.uid.uidByte[i]<0x10) uid+="0";
    uid+=String(rfid.uid.uidByte[i],HEX);
  }
  uid.toUpperCase();
  Serial.println(uid);

  if(isAllowed(uid)){
    showOLED("AKSES","DITERIMA");
    beep(2000,100); beep(2500,100);

    door.write(90);     // buka pintu
    delay(3000);
    door.write(0);      // tutup pintu
  }else{
    showOLED("AKSES","DITOLAK");
    beep(800,400);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000);
}
