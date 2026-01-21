const uint32_t BUZZER_ON_TIME = 2000;
const uint32_t BUZZER_OFF_TIME = 1000;

#include "process_control.h"
#include "config.h"
#include "telegram_bot.h"

extern hw_timer_t * timer;

bool buzzerCycleActive = false;
bool buzzerCurrentState = false;
uint32_t buzzerLastChange = 0;

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
// ЗАПУСК ЦИКЛИЧЕСКОГО ЗУММЕРА
// =====================================================
void startBuzzerCycle() {
  if (!buzzerCycleActive && alarmEnabled) {
    buzzerCycleActive = true;
    buzzerCurrentState = false;
    buzzerLastChange = millis();
    Serial.println("Buzzer cycle STARTED");
  }
}

// =====================================================
// ОСТАНОВКА ЦИКЛИЧЕСКОГО ЗУММЕРА
// =====================================================
void stopBuzzerCycle() {
  if (buzzerCycleActive) {
    buzzerCycleActive = false;
    digitalWrite(ALARM_PIN_33, LOW);
    buzzerCurrentState = false;
    Serial.println("Buzzer cycle STOPPED");
  }
}

// =====================================================
// ОБНОВЛЕНИЕ ЦИКЛИЧЕСКОГО ЗУММЕРА (вызывать в loop)
// =====================================================
void updateBuzzerCycle() {
  if (!buzzerCycleActive) return;
  
  uint32_t currentTime = millis();
  
  // Зуммер сейчас включен
  if (buzzerCurrentState) {
    if (currentTime - buzzerLastChange >= BUZZER_ON_TIME) {
      // Время истекло - выключаем
      digitalWrite(ALARM_PIN_33, LOW);
      buzzerCurrentState = false;
      buzzerLastChange = currentTime;
      Serial.println("Buzzer: OFF (pause 1 sec)");
    }
  }
  // Зуммер сейчас выключен
  else {
    if (currentTime - buzzerLastChange >= BUZZER_OFF_TIME) {
      // Пауза истекла - включаем
      digitalWrite(ALARM_PIN_33, HIGH);
      buzzerCurrentState = true;
      buzzerLastChange = currentTime;
      Serial.println("Buzzer: ON (2 sec)");
    }
  }
}

// =====================================================
// ЛОГИКА КОНТРОЛЯ ТЕМПЕРАТУРЫ (ИЗМЕНЕННАЯ ВЕРСИЯ)
// =====================================================
void checkProcessLimits() {
  if (!processStarted || sensorErrorActive) {
    stopBuzzerCycle();  // ДОБАВЛЕНО: останавливаем зуммер при ошибке или остановке процесса
    return;
  }
  
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
    
    // ИЗМЕНЕНО: Запускаем циклический зуммер если разрешен
    if (alarmEnabled && !buzzerCycleActive) {
      startBuzzerCycle();
      sendBotMessage("🔔 Зуммер ВКЛЮЧЕН (циклический режим)", chatID);
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
    
    // ИЗМЕНЕНО: Останавливаем циклический зуммер
    if (buzzerCycleActive) {
      stopBuzzerCycle();
      sendBotMessage("🔕 Зуммер ВЫКЛЮЧЕН", chatID);
    }
  }
}
