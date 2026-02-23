#include <Wire.h>
#include <U8g2lib.h>
#include <RTClib.h>

// ================= PIN =================
#define SDA_PIN     21
#define SCL_PIN     22
#define BUZZER_PIN  25
#define TOUCH_PIN   32   // TTP223
#define SOUND_PIN   34   // Sound sensor

// ================= OLED & RTC =================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SDA_PIN, SCL_PIN);
RTC_DS3231 rtc;

// ================= EKSPRESI =================
enum Expression { NORMAL, ANGRY, LOVE, SLEEPY, WINK };
Expression currentExp = NORMAL;
Expression nextExp = NORMAL;

// ================= VARIABEL KONTROL =================
bool manualMode = false;
unsigned long manualTimer = 0;
float eyeOpen = 1.0;
bool transitioning = false;
bool closing = true;
unsigned long lastRandom = 0;
float eyePos = 0;       // offset horizontal mata
float targetPos = 0;    // target posisi mata

bool lastTouch = LOW;
bool lastSound = LOW;

// ================= ALARM SHOLAT & PAGI =================
// Set jam & menit sholat bebas di sini
struct SholatTime { int hour; int minute; };
SholatTime sholatTimes[5] = { {4,30},{11,45},{15,15},{18,0},{19,15} };
bool sholatTriggered[5] = {false,false,false,false,false};

// Set alarm pagi bebas
int wakeHour = 7, wakeMinute = 0;
bool wakeTriggered = false;

// ================= BUZZER =================
void soundLove() {
  tone(BUZZER_PIN, 1200, 80);
  delay(90);
  tone(BUZZER_PIN, 1600, 80);
}

void soundAngry() {
  tone(BUZZER_PIN, 400, 120);
}

void alarmBuzzer(int duration, int baseFreq) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    int f = baseFreq + random(-200, 200);
    tone(BUZZER_PIN, f, 100);
    delay(100);
  }
}

// ================= DRAW WAJAH =================
void drawNormal(float open) {
  int r = max(2, (int)(10 * open));
  u8g2.drawDisc(44 + eyePos, 32, r);
  u8g2.drawDisc(84 + eyePos, 32, r);
}

void drawLove(float open) {
  float t = millis() * 0.004; 
  int yOffset = (int)(sin(t) * 4); // naik-turun tipis
  int y = (1.0 - open) * 6 + yOffset;

  u8g2.drawDisc(40, 30 + y, 6);
  u8g2.drawDisc(48, 30 + y, 6);
  u8g2.drawTriangle(34, 32 + y, 54, 32 + y, 44, 45 + y - 2);

  u8g2.drawDisc(80, 30 + y, 6);
  u8g2.drawDisc(88, 30 + y, 6);
  u8g2.drawTriangle(74, 32 + y, 94, 32 + y, 84, 45 + y - 2);
}

void drawAngry(float open) {
  int h = max(2, (int)(12 * open));
  u8g2.drawTriangle(30, 38, 60, 38 - h, 60, 38);
  u8g2.drawTriangle(98, 38, 68, 38 - h, 68, 38);
}

void drawSleepy(float open) {
  int h = max(1, (int)(6 * open));
  u8g2.drawEllipse(44 + eyePos, 34, 12, h);
  u8g2.drawEllipse(84 + eyePos, 34, 12, h);
}

void drawWink(float open) {
  int r = max(2, (int)(10 * open));
  u8g2.drawDisc(44 + eyePos, 32, r);
  u8g2.drawBox(74, 32, 20, 4);
}

// ================= JAM =================
void drawClock() {
  DateTime now = rtc.now();
  char buf[6];
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(96, 8, buf);
}

// ================= SETUP =================
void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);

  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  rtc.begin();
  randomSeed(analogRead(0));

  // ===== SET JAM SESUAI KOMPUTER SAAT UPLOAD =====
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

// ================= LOOP =================
void loop() {
  unsigned long now = millis();
  DateTime nowTime = rtc.now();

  // ===== SMOOTH EYE MOVEMENT =====
  if (!manualMode && !transitioning) {
    if (currentExp == NORMAL) { 
      if (random(0,100) < 2) targetPos = random(-10, 11);
      eyePos += (targetPos - eyePos) * 0.05; // easing mata
    } else {
      eyePos = 0; // ekspresi manual tetap di tengah
    }
  }

  // ===== TTP223 TOUCH (LOVE) =====
  bool touchNow = digitalRead(TOUCH_PIN);
  if (touchNow == HIGH && lastTouch == LOW) {
    currentExp = LOVE;
    manualMode = true;
    manualTimer = now;
    soundLove();
  }
  lastTouch = touchNow;

  // ===== SOUND SENSOR (ANGRY) =====
  bool soundNow = digitalRead(SOUND_PIN);
  if (soundNow == HIGH && lastSound == LOW && !manualMode) {
    currentExp = ANGRY;
    manualMode = true;
    manualTimer = now;
    soundAngry();
  }
  lastSound = soundNow;

  // ===== MANUAL MODE TIMEOUT =====
  if (manualMode) {
    if (currentExp == LOVE && now - manualTimer > 8000) {
      manualMode = false;
      currentExp = NORMAL;
    }
    if (currentExp == ANGRY && now - manualTimer > 2000) {
      manualMode = false;
      currentExp = NORMAL;
    }
  }

  // ===== RANDOM EKSPRESI (HANYA SAAT IDLE, LOVE DIHAPUS) =====
  if (!manualMode && !transitioning && now - lastRandom > 6000) {
    nextExp = (Expression)random(0, 4); // NORMAL, ANGRY, SLEEPY, WINK
    transitioning = true;
    closing = true;
    lastRandom = now;
  }

  // ===== TRANSISI TUTUP-BUKA MATA =====
  if (transitioning) {
    float delta = 0.06;
    if (closing) {
      eyeOpen -= delta * (eyeOpen + 0.2);
      if (eyeOpen <= 0) {
        eyeOpen = 0;
        currentExp = nextExp;
        closing = false;
      }
    } else {
      eyeOpen += delta * (1.0 - eyeOpen);
      if (eyeOpen >= 0.99) {
        eyeOpen = 1.0;
        transitioning = false;
      }
    }
  }

  // ===== ALARM SHOLAT 5 WAKTU (BISA DISETTING DI SHOLATTIMES) =====
  for (int i=0;i<5;i++){
    if(!sholatTriggered[i] && nowTime.hour()==sholatTimes[i].hour && nowTime.minute()==sholatTimes[i].minute){
      currentExp = WINK;
      manualMode = true;
      manualTimer = millis();
      sholatTriggered[i]=true;
      alarmBuzzer(20000, 1000);
    }
    if(sholatTriggered[i] && nowTime.minute()!=sholatTimes[i].minute){
      sholatTriggered[i]=false;
    }
  }

  // ===== ALARM PAGI (BISA DISETTING DI wakeHour/wakeMinute) =====
  if(!wakeTriggered && nowTime.hour()==wakeHour && nowTime.minute()==wakeMinute){
    currentExp = SLEEPY;
    manualMode = true;
    manualTimer = millis();
    wakeTriggered = true;
    alarmBuzzer(20000, 800);
  }
  if(wakeTriggered && (nowTime.hour()!=wakeHour || nowTime.minute()!=wakeMinute)){
    wakeTriggered=false;
  }

  // ===== DRAW OLED =====
  u8g2.clearBuffer();
  switch (currentExp){
    case NORMAL: drawNormal(eyeOpen); break;
    case LOVE:   drawLove(eyeOpen); break;
    case ANGRY:  drawAngry(eyeOpen); break;
    case SLEEPY: drawSleepy(eyeOpen); break;
    case WINK:   drawWink(eyeOpen); break;
  }
  drawClock();
  u8g2.sendBuffer();
  delay(16);
}