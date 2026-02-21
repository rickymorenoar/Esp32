#include <U8g2lib.h>
#include <Wire.h>

#define SDA_PIN 6
#define SCL_PIN 7
#define SOUND_PIN 0

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL_PIN, SDA_PIN, U8X8_PIN_NONE);

enum Expression { NORMAL, ANGRY, LOVE };
Expression currentExp = NORMAL;
Expression nextExp = NORMAL;

float eyePos = 0;
float targetPos = 0;
float eyeOpen = 1.0;
bool transitioning = false;
bool closing = true;
unsigned long lastChange = 0;
int loveHeartY = 0;

const int SOUND_HIGH = 600;

// Kata-kata penyemangat
const char* messages[] = {
  "Keep going!",
  "You can do it!",
  "Never give up!",
  "Stay strong!",
  "Smile :)",
  "Fight on!"
};

// ===== DRAW =====
void drawNormal(float offset, float open){
  int r = 10*open; if(r<2) r=2;
  u8g2.drawDisc(44+offset,32,r);
  u8g2.drawDisc(84+offset,32,r);

  // Tampilkan kata-kata penyemangat random
  u8g2.setFont(u8g2_font_5x7_tr);
  int idx = random(0, sizeof(messages)/sizeof(messages[0]));
  u8g2.drawStr(20,60, messages[idx]);
}

void drawAngry(float open){
  int h = 12*open; if(h<2) h=2;
  u8g2.drawTriangle(30,38,60,38-h,60,38);
  u8g2.drawTriangle(98,38,68,38-h,68,38);
}

void drawLove(float open){
  int y = (1.0 - open)*8;
  u8g2.drawDisc(40,30+y,6);
  u8g2.drawDisc(48,30+y,6);
  u8g2.drawDisc(80,30+y,6);
  u8g2.drawDisc(88,30+y,6);
  u8g2.drawTriangle(34,32+y,54,32+y,44,45+y);
  u8g2.drawTriangle(74,32+y,94,32+y,84,45+y);
  u8g2.drawTriangle(44,28-loveHeartY,42,32-loveHeartY,46,32-loveHeartY);
  u8g2.drawTriangle(84,28-loveHeartY,82,32-loveHeartY,86,32-loveHeartY);
  loveHeartY++; if(loveHeartY>20) loveHeartY=0;
}

// ===== SETUP =====
void setup(){
  u8g2.begin();
  pinMode(SOUND_PIN,INPUT);
  randomSeed(analogRead(0));
}

// ===== LOOP =====
void loop(){
  unsigned long now = millis();

  // ==== Baca sensor suara ====
  int soundValue = analogRead(SOUND_PIN);
  if(soundValue>=SOUND_HIGH){
    nextExp = ANGRY;
    transitioning=true;
    closing=true;
    lastChange=now;
  }

  // ==== Mata NORMAL bergerak smooth kiri-kanan terus menerus ====
  if(currentExp==NORMAL){
    targetPos = 15*sin(millis()*0.002); // gerak halus kiri-kanan
    eyePos += (targetPos-eyePos)*0.1;
  }

  // ==== Kedip otomatis ====
  if(!transitioning && now-lastChange>random(3000,6000)){
    nextExp=currentExp;
    transitioning=true;
    closing=true;
    lastChange=now;
  }

  // ==== Transisi tutup-buka ====
  if(transitioning){
    if(closing){ 
      eyeOpen-=0.06; 
      if(eyeOpen<=0){ 
        eyeOpen=0; 
        currentExp=nextExp; 
        closing=false; 
        // Kalau ANGRY selesai, ganti LOVE
        if(currentExp==ANGRY){
          nextExp=LOVE;
          transitioning=true;
          closing=true;
          lastChange=now;
        }
      } 
    } else { 
      eyeOpen+=0.06; 
      if(eyeOpen>=1){ 
        eyeOpen=1; 
        transitioning=false; 
      } 
    }
  }

  // ==== Balik ke NORMAL setelah LOVE muncul 2 detik ====
  if(currentExp==LOVE && now-lastChange>2000){
    nextExp=NORMAL;
    transitioning=true;
    closing=true;
    lastChange=now;
  }

  // ==== Gambar ke OLED ====
  u8g2.clearBuffer();
  switch(currentExp){
    case NORMAL: drawNormal(eyePos,eyeOpen); break;
    case ANGRY: drawAngry(eyeOpen); break;
    case LOVE: drawLove(eyeOpen); break;
  }
  u8g2.sendBuffer();
  delay(16);
}
