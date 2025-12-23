#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define W 128
#define H 64
Adafruit_SSD1306 display(W, H, &Wire, -1);

// BUTTON
#define UP     26
#define DOWN   27
#define FIRE   13
#define RESET  14

// PLAYER
int px = 5, py = 28;

// BULLET
bool bullet = false;
int bx, by;

// ASTEROID
int ax, ay;
int aspeed = 2;

// BOSS
bool bossMode = false;
int bossX = 100, bossY = 20;
int bossHP = 20;
int bossDir = 1;

// GAME
int score = 0;
bool gameOver = false;
bool win = false;

// BUTTON LATCH
bool fireLast = HIGH;
bool resetLast = HIGH;

void resetAsteroid() {
  ax = W;
  ay = random(0, H - 8);
}

void resetGame() {
  px = 5; py = 28;
  score = 0;
  bossMode = false;
  bossHP = 20;
  gameOver = false;
  win = false;
  bullet = false;
  resetAsteroid();
}

void setup() {
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(FIRE, INPUT_PULLUP);
  pinMode(RESET, INPUT_PULLUP);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);

  randomSeed(millis());
  resetAsteroid();
}

void loop() {
  bool fireNow = digitalRead(FIRE);
  bool resetNow = digitalRead(RESET);

  // ===== GAME OVER / WIN =====
  if (gameOver || win) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 20);
    display.println(win ? "YOU WIN!" : "GAME OVER");
    display.setCursor(30, 35);
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

  // ===== MOVE PLAYER =====
  if (digitalRead(UP) == LOW && py > 0) py -= 3;
  if (digitalRead(DOWN) == LOW && py < H - 8) py += 3;

  // ===== FIRE =====
  if (fireLast == HIGH && fireNow == LOW && !bullet) {
    bullet = true;
    bx = px + 8;
    by = py + 3;
  }
  fireLast = fireNow;

  if (bullet) {
    bx += 4;
    if (bx > W) bullet = false;
  }

  // ===== ASTEROID MODE =====
  if (!bossMode) {
    ax -= aspeed;
    if (ax < -8) resetAsteroid();

    // HIT ASTEROID
    if (bullet &&
        bx > ax && bx < ax + 8 &&
        by > ay && by < ay + 8) {
      bullet = false;
      score++;
      resetAsteroid();
    }

    // HIT PLAYER
    if (px + 8 > ax &&
        py + 8 > ay &&
        py < ay + 8) {
      gameOver = true;
    }

    if (score >= 10) bossMode = true;
  }

  // ===== BOSS MODE =====
  else {
    bossY += bossDir;
    if (bossY <= 0 || bossY >= H - 16) bossDir = -bossDir;

    if (bullet &&
        bx > bossX && bx < bossX + 16 &&
        by > bossY && by < bossY + 16) {
      bullet = false;
      bossHP--;
      if (bossHP <= 0) win = true;
    }

    if (px + 8 > bossX &&
        py + 8 > bossY &&
        py < bossY + 16) {
      gameOver = true;
    }
  }

  // ===== DRAW =====
  display.clearDisplay();
  display.fillRect(px, py, 8, 8, SSD1306_WHITE); // player

  if (bullet)
    display.fillRect(bx, by, 3, 2, SSD1306_WHITE);

  if (!bossMode)
    display.fillRect(ax, ay, 8, 8, SSD1306_WHITE);
  else {
    display.drawRect(bossX, bossY, 16, 16, SSD1306_WHITE);
    display.setCursor(80, 0);
    display.print("HP:");
    display.print(bossHP);
  }

  display.setCursor(0, 0);
  display.print(score);

  display.display();
  delay(20);
}
