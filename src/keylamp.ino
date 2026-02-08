int pushButton = 2;

struct RGB {
  byte r;
  byte g;
  byte b;
};

// Яркость 0..255
int BRIGHT = 255;

RGB colors[] = {
  {0, 0, 0},               // 0 - черный
  {BRIGHT, 0, 0},          // 1 - красный
  {0, BRIGHT, 0},          // 2 - зелёный
  {0, 0, BRIGHT},           // 3 - синий
  {0, 0, 0},               // 4 - зарезервировано
  {0, 0, 0},               // 5 - зарезервировано
  {0, 0, 0},               // 6 - зарезервировано
  {0, 0, 0},               // 7 - зарезервировано
  {10, 10, 10},            // 8 - тусклый белый
  {BRIGHT, BRIGHT, BRIGHT} // 9 - белый
};

// Текущий и целевой цвет
RGB currentColor = {0, 0, 0};
RGB targetColor  = {0, 0, 0};

RGB fadeFrom; // исходный цвет для текущего fade

// Авто-выключение
unsigned long lastActionTime = 0; // Время последней команды
bool isIdle = false;              // Флаг авто-выключения
const unsigned long IDLE_TIMEOUT = 3600000; // 1 час

// Неблокирующий fade
int fadeSteps = 0;           
int fadeStep = 0;            
int fadeDelayMs = 0;

unsigned long lastFadeTime = 0;
bool isFading = false;

void setup() {
  pinMode(pushButton, INPUT);
  Serial.begin(9600);

  // тестовая анимация при старте
  for (int i = 3; i >= 0; i--) {
    startFade(currentColor, colors[i], 10, 5);
    while (isFading) updateFade(); // дождаться окончания fade
    currentColor = colors[i];
  }
  startFade(currentColor, colors[8], 10, 5);

  // Инициализация пинов PWM
  analogWrite(9, currentColor.r);
  analogWrite(10, currentColor.g);
  analogWrite(11, currentColor.b);

  lastActionTime = millis();
}

// Запуск плавного перехода
void startFade(RGB from, RGB to, int steps, int delayMs) {
  fadeFrom = from;       // сохраняем начальный цвет
  targetColor = to;
  fadeSteps = steps;
  fadeStep = 0;
  fadeDelayMs = delayMs;
  lastFadeTime = millis();
  isFading = true;
}

// Обновление цвета на каждом шаге (не блокирующее)
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

    analogWrite(9, currentColor.r);
    analogWrite(10, currentColor.g);
    analogWrite(11, currentColor.b);

    lastFadeTime = now;
  }
}

void loop() {
  // Проверка Serial
  if (Serial.available()) {
    char c = Serial.read();
    lastActionTime = millis();  // сброс таймера активности
    isIdle = false;             // выход из idle

    if (c == '?') {
      Serial.print("ARDUINO_OK");
    }

    if (c >= '0' && c <= '9') {
      byte command = c - '0';
      startFade(currentColor, colors[command], 32, 10);
    }
  }

  // Обновление fade
  updateFade();

  // Авто-выключение по таймауту
  if (!isIdle && millis() - lastActionTime >= IDLE_TIMEOUT) {
    startFade(currentColor, colors[8], 254, 100);
    isIdle = true;
  }
}
