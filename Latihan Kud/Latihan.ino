#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define W 128
#define H 64
Adafruit_SSD1306 display(W, H, &Wire, -1);

// BUTTON
#define LEFT   26
#define RIGHT  27
#define GAS    13
#define RESET  14

// PLAYER
int px = 60;
int py = 50;

// ENEMY
int ex = 0;
int ey = -10;
int speed = 2;

// GAME
bool gameOver = false;
int score = 0;
bool resetLast = HIGH;

void spawnEnemy() {
  ex = random(0, W - 10);
  ey = -10;
}

void resetGame() {
  px = 60;
  score = 0;
  speed = 2;
  gameOver = false;
  spawnEnemy();
}

void setup() {
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(GAS, INPUT_PULLUP);
  pinMode(RESET, INPUT_PULLUP);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);

  randomSeed(millis());
  spawnEnemy();
}

void loop() {
  bool resetNow = digitalRead(RESET);

  // ===== GAME OVER =====
  if (gameOver) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(30, 20);
    display.println("GAME OVER");
    display.setCursor(25, 35);
    display.print("Score: ");
    display.print(score);
    display.setCursor(10, 50);
    display.print("PRESS RESET");
    display.display();

    if (resetLast == HIGH && resetNow == LOW) {
      resetGame();
      delay(300);
    }
    resetLast = resetNow;
    return;
  }

  // ===== CONTROL =====
  if (digitalRead(LEFT) == LOW && px > 0) px -= 3;
  if (digitalRead(RIGHT) == LOW && px < W - 10) px += 3;

  if (digitalRead(GAS) == LOW) ey += speed + 1;
  else ey += speed;

  // ===== ENEMY LOGIC =====
  if (ey > H) {
    score++;
    speed = 2 + score / 5; // makin cepat
    spawnEnemy();
  }

  // COLLISION
  if (px < ex + 10 &&
      px + 10 > ex &&
      py < ey + 10 &&
      py + 10 > ey) {
    gameOver = true;
  }

  // ===== DRAW =====
  display.clearDisplay();

  // ROAD LINE
  for (int i = 0; i < H; i += 6)
    display.drawPixel(W / 2, i, SSD1306_WHITE);

  // PLAYER CAR
  display.fillRect(px, py, 10, 10, SSD1306_WHITE);

  // ENEMY CAR
  display.fillRect(ex, ey, 10, 10, SSD1306_WHITE);

  // SCORE
  display.setCursor(0, 0);
  display.print(score);

  display.display();
  delay(20);
}
