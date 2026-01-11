#include "process_control.h"
#include "config.h"
#include "telegram_bot.h"

extern hw_timer_t * timer;

// =====================================================
// ПРЕРЫВАНИЕ ТАЙМЕРА (каждую секунду)
// =====================================================
void IRAM_ATTR onTimer() {
  if (timerActive && timerSeconds > 0) {
    timerSeconds--;
    if (timerSeconds == 0) {
      timerActive = false;
      timerFinished = true;
      showTimerZero = true; // Показать 0 на дисплее
    }
  }
}

// =====================================================
// ИНИЦИАЛИЗАЦИЯ АППАРАТНОГО ТАЙМЕРА
// =====================================================
void initHardwareTimer() {
  timer = timerBegin(1000000);              // 1 МГц (1 тик = 1 мкс)
  timerAttachInterrupt(timer, &onTimer);    // Привязка функции
  timerAlarm(timer, 1000000, true, 0);      // 1 000 000 тиков = 1 сек, авто-перезапуск
  timerStart(timer);
  Serial.println("Hardware timer initialized");
}

// =====================================================
// ОБРАБОТКА ЗАВЕРШЕНИЯ ТАЙМЕРА
// =====================================================
void handleTimerFinished() {
  if (timerFinished) {
    timerFinished = false;
    timerZeroShowTime = millis(); // Запоминаем время показа 0
    Serial.println("TIMER FINISHED! timerSeconds = " + String(timerSeconds));
    sendBotMessage("🔔 ТАЙМЕР: Время истекло!", chatID);
  }
  
  // Проверяем, прошла ли секунда после показа 0
  if (showTimerZero && millis() - timerZeroShowTime >= TIMER_ZERO_DISPLAY_DURATION) {
    showTimerZero = false;
    lastTimerSeconds = -1; // Сбрасываем, чтобы обновить дисплей на время
  }
}

// =====================================================
// ЛОГИКА КОНТРОЛЯ ТЕМПЕРАТУРЫ (процесс /start)
// =====================================================
void checkProcessLimits() {
  if (!processStarted || sensorErrorActive) return;
  
  float threshold = myTmpMax + tempDev;
  
  // ===== ПРОВЕРКА ПРЕВЫШЕНИЯ ТЕМПЕРАТУРЫ =====
  if (myTmpCur >= threshold) {
    
    // Закрыть клапан (D26 → HIGH)
    if (digitalRead(VALVE_PIN_26) == LOW) {
      digitalWrite(VALVE_PIN_26, HIGH);
      Serial.println("PROCESS: Temperature exceeded! Valve CLOSED (HIGH on D26)");
      sendBotMessage("🚨 ПРЕВЫШЕНИЕ ТЕМПЕРАТУРЫ!\n🌡️ Текущая: " + String(myTmpCur, 1) + 
                     "°C\n🎯 Порог: " + String(threshold, 1) + 
                     "°C\n🚰 Клапан ЗАКРЫТ", chatID);
    }
    
    // Включить зуммер если разрешен (D33 → HIGH)
    if (alarmEnabled && digitalRead(ALARM_PIN_33) == LOW) {
      digitalWrite(ALARM_PIN_33, HIGH);
      Serial.println("PROCESS: Alarm activated (HIGH on D33)");
      sendBotMessage("🔔 Зуммер ВКЛЮЧЕН", chatID);
    }
  }
  
  // ===== ПРОВЕРКА НОРМАЛИЗАЦИИ ТЕМПЕРАТУРЫ =====
  else if (myTmpCur <= myTmpMax) {
    
    // Открыть клапан (D26 → LOW)
    if (digitalRead(VALVE_PIN_26) == HIGH) {
      digitalWrite(VALVE_PIN_26, LOW);
      Serial.println("PROCESS: Temperature normalized! Valve OPEN (LOW on D26)");
      sendBotMessage("✅ ТЕМПЕРАТУРА В НОРМЕ\n🌡️ Текущая: " + String(myTmpCur, 1) + 
                     "°C\n🚰 Клапан ОТКРЫТ", chatID);
    }
    
    // Выключить зуммер (D33 → LOW)
    if (digitalRead(ALARM_PIN_33) == HIGH) {
      digitalWrite(ALARM_PIN_33, LOW);
      Serial.println("PROCESS: Alarm deactivated (LOW on D33)");
    }
  }
}
