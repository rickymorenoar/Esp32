#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== BUTTON =====
#define P1_UP     26
#define P1_DOWN   25
#define P2_UP     13
#define P2_DOWN   12

// ===== GAME =====
#define MAX_SCORE 5

int paddleW = 3;
int paddleH = 16;

int p1Y = 24;
int p2Y = 24;

int ballX = 64;
int ballY = 32;
int ballDX = 2;
int ballDY = 2;

int score1 = 0;
int score2 = 0;

bool gameOver = false;

void setup() {
  pinMode(P1_UP, INPUT_PULLUP);
  pinMode(P1_DOWN, INPUT_PULLUP);
  pinMode(P2_UP, INPUT_PULLUP);
  pinMode(P2_DOWN, INPUT_PULLUP);

  Wire.begin(21, 22); // SDA, SCL
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1); // OLED tidak terdeteksi
  }
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  if (gameOver) {
    showWinner();
    if (digitalRead(P1_UP) == LOW) { // restart
      resetGame();
      delay(300);
    }
    return;
  }

  // PLAYER 1
  if (digitalRead(P1_UP) == LOW && p1Y > 0) p1Y -= 3;
  if (digitalRead(P1_DOWN) == LOW && p1Y < SCREEN_HEIGHT - paddleH) p1Y += 3;

  // PLAYER 2
  if (digitalRead(P2_UP) == LOW && p2Y > 0) p2Y -= 3;
  if (digitalRead(P2_DOWN) == LOW && p2Y < SCREEN_HEIGHT - paddleH) p2Y += 3;

  // BALL
  ballX += ballDX;
  ballY += ballDY;

  if (ballY <= 0 || ballY >= SCREEN_HEIGHT - 2) ballDY = -ballDY;

  if (ballX <= paddleW &&
      ballY >= p1Y && ballY <= p1Y + paddleH) ballDX = -ballDX;

  if (ballX >= SCREEN_WIDTH - paddleW - 2 &&
      ballY >= p2Y && ballY <= p2Y + paddleH) ballDX = -ballDX;

  if (ballX < 0) {
    score2++;
    resetBall();
  }
  if (ballX > SCREEN_WIDTH) {
    score1++;
    resetBall();
  }

  if (score1 >= MAX_SCORE || score2 >= MAX_SCORE) {
    gameOver = true;
  }

  drawGame();
  delay(20);
}

void resetBall() {
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  ballDX = -ballDX;
}

void resetGame() {
  score1 = 0;
  score2 = 0;
  p1Y = 24;
  p2Y = 24;
  gameOver = false;
  resetBall();
}

void showWinner() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(15, 20);
  if (score1 > score2) display.print("LEFT PLAYER WINS");
  else display.print("RIGHT PLAYER WINS");
  display.setCursor(18, 40);
  display.print("PRESS P1 UP");
  display.display();
}

void drawGame() {
  display.clearDisplay();

  for (int i = 0; i < SCREEN_HEIGHT; i += 4)
    display.drawPixel(SCREEN_WIDTH / 2, i, SSD1306_WHITE);

  display.fillRect(0, p1Y, paddleW, paddleH, SSD1306_WHITE);
  display.fillRect(SCREEN_WIDTH - paddleW, p2Y, paddleW, paddleH, SSD1306_WHITE);

  display.fillRect(ballX, ballY, 2, 2, SSD1306_WHITE);

  display.setCursor(54, 0);
  display.print(score1);
  display.print(" : ");
  display.print(score2);

  display.display();
}
