#include <U8g2lib.h>
#include <Wire.h>

#define SDA_PIN 6
#define SCL_PIN 7

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

enum Expression {
  NORMAL,
  ANGRY,
  LOVE,
  SLEEPY,
  SHOCK,
  WINK
};

Expression currentExp = NORMAL;
Expression nextExp = NORMAL;

float eyePos = 0;
float targetPos = 0;
float eyeOpen = 1.0;

bool transitioning = false;
bool closing = true;

unsigned long lastChange = 0;

void drawNormal(float offset, float open) {
  int r = 10 * open;
  if (r < 2) r = 2;
  u8g2.drawDisc(44 + offset, 32, r);
  u8g2.drawDisc(84 + offset, 32, r);
}
void drawAngry(float open) {
  int h = 12 * open;
  if (h < 2) h = 2;
  u8g2.drawTriangle(30, 38, 60, 38 - h, 60, 38);
  u8g2.drawTriangle(98, 38, 68, 38 - h, 68, 38);
}

void drawLove(float open) {
  int y = (1.0 - open) * 8;
  u8g2.drawDisc(40, 30 + y, 6);
  u8g2.drawDisc(48, 30 + y, 6);
  u8g2.drawTriangle(34, 32 + y, 54, 32 + y, 44, 45 + y);

  u8g2.drawDisc(80, 30 + y, 6);
  u8g2.drawDisc(88, 30 + y, 6);
  u8g2.drawTriangle(74, 32 + y, 94, 32 + y, 84, 45 + y);
}

void drawSleepy(float open) {
  int h = 6 * open;
  if (h < 1) h = 1;
  u8g2.drawEllipse(44, 34, 12, h);
  u8g2.drawEllipse(84, 34, 12, h);
}

// ================= SHOCK =================
void drawShock(float open) {
  int r = 14 * open;
  if (r < 2) r = 2;
  u8g2.drawCircle(44, 32, r);
  u8g2.drawCircle(84, 32, r);
}

// ================= WINK =================
void drawWink(float open) {
  int r = 10 * open;
  if (r < 2) r = 2;
  u8g2.drawDisc(44, 32, r);   // kiri normal
  u8g2.drawBox(74, 32, 20, 4); // kanan kedip
}

// ================= SETUP =================
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  randomSeed(analogRead(0));
}

// ================= LOOP =================
void loop() {

  unsigned long now = millis();

  // Smooth eye movement
  if (!transitioning && random(0,100) < 2) {
    targetPos = random(-10, 11);
  }
  eyePos += (targetPos - eyePos) * 0.06;

  // Ganti ekspresi random natural
  if (!transitioning && now - lastChange > random(4000, 7000)) {
    nextExp = (Expression)random(0,6);
    transitioning = true;
    closing = true;
    lastChange = now;
  }

  // Transisi smooth tutup-buka
  if (transitioning) {

    if (closing) {
      eyeOpen -= 0.06;
      if (eyeOpen <= 0) {
        eyeOpen = 0;
        currentExp = nextExp;
        closing = false;
      }
    } else {
      eyeOpen += 0.06;
      if (eyeOpen >= 1) {
        eyeOpen = 1;
        transitioning = false;
      }
    }
  }

  u8g2.clearBuffer();

  switch (currentExp) {
    case NORMAL: drawNormal(eyePos, eyeOpen); break;
    case ANGRY:  drawAngry(eyeOpen); break;
    case LOVE:   drawLove(eyeOpen); break;
    case SLEEPY: drawSleepy(eyeOpen); break;
    case SHOCK:  drawShock(eyeOpen); break;
    case WINK:   drawWink(eyeOpen); break;
  }

  u8g2.sendBuffer();
  delay(16); 
}
