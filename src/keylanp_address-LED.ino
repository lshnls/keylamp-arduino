# For WS2812B LED

#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    1
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define BUTTON_PIN  2

CRGB leds[NUM_LEDS];

// Массив цветов
CRGB colors[] = {
  CRGB::Black,          // 0
  CRGB::Red,            // 1
  CRGB::Green,          // 2
  CRGB::Blue,           // 3
  CRGB::Black,          // 4
  CRGB::Black,          // 5
  CRGB::Black,          // 6
  CRGB::Black,          // 7
  CRGB(10,10,10),       // 8 - тусклый белый
  CRGB::White           // 9 - яркий белый
};

CRGB currentColor = CRGB::Black;
CRGB targetColor = CRGB::Black;
CRGB fadeFrom;

// Авто-выключение
unsigned long lastActionTime = 0; // Время последней команды
bool isIdle = false;              // Флаг авто-выключения
const unsigned long IDLE_TIMEOUT = 36000; // 1 час

int fadeSteps = 0;
int fadeStep = 0;
int fadeDelayMs = 0;
bool isFading = false;
unsigned long lastFadeTime = 0;

void startFade(CRGB from, CRGB to, int steps, int delayMs) {
  fadeFrom = from;
  targetColor = to;
  fadeSteps = steps;
  fadeStep = 0;
  fadeDelayMs = delayMs;
  lastFadeTime = millis();
  isFading = true;
}

void updateFade() {
  if (!isFading) return;

  unsigned long now = millis();
  if (now - lastFadeTime >= fadeDelayMs) {
    fadeStep++;
    if (fadeStep > fadeSteps) {
      currentColor = targetColor;
      isFading = false;
    } else {
      currentColor.r = map(fadeStep, 0, fadeSteps, fadeFrom.r, targetColor.r);
      currentColor.g = map(fadeStep, 0, fadeSteps, fadeFrom.g, targetColor.g);
      currentColor.b = map(fadeStep, 0, fadeSteps, fadeFrom.b, targetColor.b);
    }
    leds[0] = currentColor;
    FastLED.show();
    lastFadeTime = now;
  }
}

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // стартовая анимация
  for (int i = 3; i >= 0; i--) {
    startFade(currentColor, colors[i], 10, 5);
    while (isFading) updateFade();
    currentColor = colors[i];
  }
  startFade(currentColor, colors[8], 10, 5);

  lastActionTime = millis();
}

void loop() {
  // считываем Serial для управления цветом
  if (Serial.available()) {
    char c = Serial.read();
    lastActionTime = millis();  // сброс таймера активности
    isIdle = false;             // выход из idle
  
    if (c == '?') {
      Serial.print("ARDUINO_OK");
    }
    if (c >= '0' && c <= '9') {
      startFade(currentColor, colors[c - '0'], 32, 10);
    }
  }

  updateFade();
  // Авто-выключение по таймауту
  if (!isIdle && millis() - lastActionTime >= IDLE_TIMEOUT) {
    startFade(currentColor, colors[8], 254, 100);
    isIdle = true;
  }

}