#include "display_control.h"
#include "config.h"

extern Disp7219<1> disp;
extern bool apMode;

// Кеш для дисплея
static float lastDisplayTemp = -999;
static int lastDisplayMinute = -1;
static int lastTimerSeconds = -1;

// =====================================================
// ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ
// =====================================================
void initDisplay() {
  disp.brightness(DISPLAY_BRIGHTNESS);
  disp.clear();
  disp.update();
  Serial.println("Display initialized");
}

// =====================================================
// ПОКАЗАТЬ СООБЩЕНИЕ СБРОСА
// =====================================================
void showResetMessage() {
  disp.clear();
  disp.setCursor(0); disp.print("r");
  disp.setCursor(1); disp.print("5");
  disp.setCursor(2); disp.print("E");
  disp.setCursor(3); disp.print("t");
  disp.update();
}

// =====================================================
// ПОКАЗАТЬ РЕЖИМ AP
// =====================================================
void showAPMode() {
  static uint32_t apBlinkTimer = 0;
  static bool apBlinkState = false;
  
  if(millis() - apBlinkTimer > 500) {
    apBlinkTimer = millis();
    apBlinkState = !apBlinkState;
    disp.clear();
    if(apBlinkState) {
      disp.setCursor(0); disp.print("A");
      disp.setCursor(1); disp.print("P");
    }
    disp.update();
  }
}

// =====================================================
// ОБНОВЛЕНИЕ ДИСПЛЕЯ
// =====================================================
void updateDisplay() {
  // Режим AP
  if(apMode) {
    showAPMode();
    return;
  }
  
  struct tm timeinfo;
  bool timeValid = getLocalTime(&timeinfo);
  int currentMinute = timeValid ? (timeinfo.tm_hour * 60 + timeinfo.tm_min) : -1;
  
  // Проверка необходимости обновления
  bool needUpdate = false;
  
  if (abs(myTmpCur - lastDisplayTemp) >= 0.1) needUpdate = true;
  if (timerActive && timerSeconds != lastTimerSeconds) needUpdate = true;
  if (!timerActive && currentMinute != lastDisplayMinute) needUpdate = true;
  if (showTimerZero) needUpdate = true; // Показать 0 таймера
  
  if (!needUpdate) return;
  
  disp.clear();
  
  // Температура (первые 3 цифры: XX.X)
  int tempInt = constrain((int)(myTmpCur * 10), 0, 999);
  disp.setCursor(0); disp.print((tempInt / 100) % 10);
  disp.setCursor(1); disp.print((tempInt / 10) % 10);
  disp.point(true); // Точка после 1-й цифры
  disp.setCursor(2); disp.print(tempInt % 10);

  // Правая часть: таймер или время
  if (timerActive || showTimerZero) {
    // Таймер (4 цифры: SSSS)
    int t = constrain(timerSeconds, 0, 9999);
    disp.setCursor(4); disp.print((t / 1000) % 10);
    disp.setCursor(5); disp.print((t / 100) % 10);
    disp.setCursor(6); disp.print((t / 10) % 10);
    disp.setCursor(7); disp.print(t % 10);
    lastTimerSeconds = timerSeconds;
  } else if (timeValid) {
    // Время (4 цифры: HH.MM)
    disp.setCursor(4); disp.print(timeinfo.tm_hour / 10);
    disp.setCursor(5); disp.print(timeinfo.tm_hour % 10);
    disp.setCursor(6); disp.print(timeinfo.tm_min / 10);
    disp.setCursor(7); disp.print(timeinfo.tm_min % 10);
    disp.point(1, true);  // Точка температуры
    disp.point(5, true);  // Точка времени
    lastDisplayMinute = currentMinute;
  }
  
  disp.update();
  lastDisplayTemp = myTmpCur;
}
