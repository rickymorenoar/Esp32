#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// BUTTON
#define BTN_UP     26
#define BTN_DOWN   27
#define BTN_FIRE   13
#define BTN_RESET  14

// PLAYER
int shipX = 5;
int shipY = 28;

// BULLET
bool bulletActive = false;
int bulletX, bulletY;

// ASTEROID
int astX = 128;
int astY = 0;
int astSpeed = 2;

// GAME
int score = 0;
bool gameOver = false;

void resetAsteroid() {
  astX = 128;
  astY = random(0, SCREEN_HEIGHT - 8);
}

void resetGame() {
  shipY = 28;
  score = 0;
  gameOver = false;
  bulletActive = false;
  resetAsteroid();
}

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_FIRE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);

  randomSeed(millis());
  resetAsteroid();
}

void loop() {
  if (gameOver) {
    display.clearDisplay();
    display.setCursor(20, 20);
    display.setTextSize(1);
    display.println("GAME OVER");
    display.setCursor(30, 35);
    display.print("Score: ");
    display.print(score);
    display.setCursor(10, 50);
    display.print("PRESS RESET");
    display.display();

    if (digitalRead(BTN_RESET) == LOW) {
      resetGame();
      delay(300);
    }
    return;
  }

  // INPUT
  if (digitalRead(BTN_UP) == LOW && shipY > 0) shipY -= 3;
  if (digitalRead(BTN_DOWN) == LOW && shipY < SCREEN_HEIGHT - 8) shipY += 3;

  if (digitalRead(BTN_FIRE) == LOW && !bulletActive) {
    bulletActive = true;
    bulletX = shipX + 8;
    bulletY = shipY + 3;
  }

  // BULLET MOVE
  if (bulletActive) {
    bulletX += 4;
    if (bulletX > SCREEN_WIDTH) bulletActive = false;
  }

  // ASTEROID MOVE
  astX -= astSpeed;
  if (astX < 0) {
    resetAsteroid();
  }

  // COLLISION BULLET vs ASTEROID
  if (bulletActive &&
      bulletX > astX && bulletX < astX + 8 &&
      bulletY > astY && bulletY < astY + 8) {
    bulletActive = false;
    score++;
    resetAsteroid();
  }

  // COLLISION SHIP vs ASTEROID
  if (shipX + 8 > astX &&
      shipY + 8 > astY &&
      shipY < astY + 8) {
    gameOver = true;
  }

  // DRAW
  display.clearDisplay();

  // Ship
  display.fillRect(shipX, shipY, 8, 8, SSD1306_WHITE);

  // Bullet
  if (bulletActive)
    display.fillRect(bulletX, bulletY, 3, 2, SSD1306_WHITE);

  // Asteroid
  display.fillRect(astX, astY, 8, 8, SSD1306_WHITE);

  // Score
  display.setCursor(90, 0);
  display.print(score);

  display.display();
  delay(20);
}
