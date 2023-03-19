// Pong Game for ESP32 with 64x64 matrix panel
// Uses the ESP32-HUB75-MatrixPanel-DMA library
// By ChatGPT

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>

const uint16_t PANEL_RES_X = 64;
const uint16_t PANEL_RES_Y = 64;
const uint16_t PANEL_CHAIN = 1;

const uint8_t PADDLE_HEIGHT = 16;
const uint8_t PADDLE_WIDTH = 2;
const uint8_t BALL_SPEED = 1;
const uint16_t WINNING_SCORE = 21;

int16_t leftPaddleY;
int16_t rightPaddleY;
int16_t ballX;
int16_t ballY;
int8_t ballSpeedX;
int8_t ballSpeedY;
int16_t leftScore;
int16_t rightScore;
float currentSpeedFactor = 1.0;

const int MAX_BALL_SPEED_Y = 3;
const int FRAME_RATE = 30;
const int MISS_PROBABILITY = 10;

MatrixPanel_I2S_DMA *dma_display = nullptr;

const uint8_t SMILEY_FACE[8] = {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100
};

void updateScore() {
  dma_display->setTextColor(dma_display->color565(0, 0, 0));
  dma_display->setCursor(PANEL_RES_X / 4, 8);
  dma_display->print(leftScore);
  dma_display->setCursor(PANEL_RES_X * 3 / 4, 8);
  dma_display->print(rightScore);

  dma_display->setTextColor(dma_display->color565(255, 255, 255));
  dma_display->setCursor(PANEL_RES_X / 4, 8);
  dma_display->print(leftScore);
  dma_display->setCursor(PANEL_RES_X * 3 / 4, 8);
  dma_display->print(rightScore);
}

void drawPaddle(int16_t x, int16_t y, uint16_t color) {
  for (int i = 0; i < PADDLE_HEIGHT; i++) {
    dma_display->drawPixel(x, y + i, color);
  }
}

void erasePaddle(int16_t x, int16_t y) {
  drawPaddle(x, y, dma_display->color565(0, 0, 0));
}

void drawSmileyFace(int16_t x, int16_t y, uint16_t color) {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (bitRead(SMILEY_FACE[i], j)) {
        dma_display->drawPixel(x + j, y + i, color);
      }
    }
  }
}

void eraseSmileyFace(int16_t x, int16_t y) {
  drawSmileyFace(x, y, dma_display->color565(0, 0, 0));
}

void clearDisplay() {
  dma_display->fillScreen(dma_display->color565(0, 0, 0));
}

void resetGame() {
  leftPaddleY = PANEL_RES_Y / 2 - PADDLE_HEIGHT / 2;
  rightPaddleY = PANEL_RES_Y / 2;
  PADDLE_HEIGHT / 2;

ballX = PANEL_RES_X / 2 - 4;
ballY = PANEL_RES_Y / 2 - 4;

ballSpeedX = random(0, 2) == 0 ? -BALL_SPEED : BALL_SPEED;
ballSpeedY = random(-BALL_SPEED, BALL_SPEED);
}

void startGame() {
leftScore = 0;
rightScore = 0;

currentSpeedFactor = 1.0;

resetGame();
}

void checkForWinnerAndReset() {
if (leftScore >= WINNING_SCORE || rightScore >= WINNING_SCORE) {
startGame();
clearDisplay();
dma_display->setTextSize(2);
dma_display->setTextColor(dma_display->color565(255, 255, 255));
dma_display->setCursor(PANEL_RES_X/2-22, PANEL_RES_Y/2-8);
dma_display->print("WIN!");
delay(3000);
dma_display->setTextSize(1);
}
}

void setup() {
Serial.begin(9600);

HUB75_I2S_CFG mxconfig(
PANEL_RES_X,
PANEL_RES_Y,
PANEL_CHAIN
);
mxconfig.gpio.e = 18;
mxconfig.clkphase = false;

dma_display = new MatrixPanel_I2S_DMA(mxconfig);
dma_display->begin();

startGame();
}

void loop() {
eraseSmileyFace(ballX, ballY);
ballX += ballSpeedX;
ballY += ballSpeedY;
drawSmileyFace(ballX, ballY, dma_display->color565(255, 0, 0));

erasePaddle(0, leftPaddleY);
erasePaddle(PANEL_RES_X - PADDLE_WIDTH, rightPaddleY);

int reactionDelay = 150;
static unsigned long lastReactionTime = 0;
if (millis() - lastReactionTime >= reactionDelay) {
lastReactionTime = millis();
if (ballSpeedX < 0) {
  if (ballY > leftPaddleY + PADDLE_HEIGHT / 2) {
    leftPaddleY += 2;
  } else {
    leftPaddleY -= 2;
  }
}

if (ballSpeedX > 0) {
  if (ballY > rightPaddleY + PADDLE_HEIGHT / 2) {
    rightPaddleY += 2;
  } else {
    rightPaddleY -= 2;
  }
}
}

leftPaddleY = constrain(leftPaddleY, 0, PANEL_RES_Y - PADDLE_HEIGHT);
rightPaddleY = constrain(rightPaddleY, 0, PANEL_RES_Y - PADDLE_HEIGHT);

drawPaddle(0, leftPaddleY, dma_display->color565(0, 255, 0));
drawPaddle(PANEL_RES_X - PADDLE_WIDTH, rightPaddleY, dma_display->color565(255, 255, 0));

if (ballY <= 0 || ballY + 8 >= PANEL_RES_Y) {
ballSpeedY = -ballSpeedY;
}

if ((ballX <= PADDLE_WIDTH && ballY + 8 >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) ||
(ballX + 8 >= PANEL_RES_X - PADDLE_WIDTH && ballY + 8 >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT)) {
ballSpeedX = -ballSpeedX;
ballSpeedY += random(-MAX_BALL_SPEED_Y, MAX_BALL_SPEED_Y + 1);
ballSpeedY = constrain(ballSpeedY, -MAX_BALL_SPEED_Y, MAX_BALL_SPEED_Y);
}

if (ballX <= 0) {
rightScore++;
currentSpeedFactor += 0.1;
resetGame();
clearDisplay();
} else if (ballX + 8 >= PANEL_RES_X) {
leftScore++;
currentSpeedFactor += 0.1;
resetGame();
clearDisplay();
}

// Check for scoring
if (ballX <= 0) {
rightScore++;
resetGame();
clearDisplay(); // Clear the display after resetting the game
} else if (ballX + 8 >= PANEL_RES_X) {
leftScore++;
resetGame();
clearDisplay(); // Clear the display after resetting the game
}

// Redraw the scores
updateScore();

// Check for a winner and reset the game if necessary
checkForWinnerAndReset();

delay(1000 / FRAME_RATE);
}
