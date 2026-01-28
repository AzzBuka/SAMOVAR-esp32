#include "process_control.h"
#include "config.h"
#include "telegram_bot.h"

extern hw_timer_t * timer;

// УБРАНО: переменные перенесены в SAMOVAR_V2.ino
// Константы остаются здесь как локальные
static const uint32_t BUZZER_ON_TIME = 2000;    // 2 секунды ВКЛ
static const uint32_t BUZZER_OFF_TIME = 1000;   // 1 секунда ВЫКЛ

// =====================================================
// ПРЕРЫВАНИЕ ТАЙМЕРА
// =====================================================
void IRAM_ATTR onTimer() {
  if (timerActive && timerSeconds > 0) {
    timerSeconds--;
    if (timerSeconds == 0) {
      timerActive = false;
      timerFinished = true;
      showTimerZero = true;
    }
  }
}

// =====================================================
// ИНИЦИАЛИЗАЦИЯ АППАРАТНОГО ТАЙМЕРА
// =====================================================
void initHardwareTimer() {
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);
  timerStart(timer);
  Serial.println("Hardware timer initialized");
}

// =====================================================
// ОБРАБОТКА ЗАВЕРШЕНИЯ ТАЙМЕРА
// =====================================================
void handleTimerFinished() {
  if (timerFinished) {
    timerFinished = false;
    timerZeroShowTime = millis();
    Serial.println("TIMER FINISHED! timerSeconds = " + String(timerSeconds));
    sendBotMessage("🔔 ТАЙМЕР: Время истекло!", chatID);
  }
  
  if (showTimerZero && millis() - timerZeroShowTime >= TIMER_ZERO_DISPLAY_DURATION) {
    showTimerZero = false;
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
// ОБНОВЛЕНИЕ ЦИКЛИЧЕСКОГО ЗУММЕРА
// =====================================================
void updateBuzzerCycle() {
  if (!buzzerCycleActive) return;
  
  uint32_t currentTime = millis();
  
  if (buzzerCurrentState) {
    if (currentTime - buzzerLastChange >= BUZZER_ON_TIME) {
      digitalWrite(ALARM_PIN_33, LOW);
      buzzerCurrentState = false;
      buzzerLastChange = currentTime;
      Serial.println("Buzzer: OFF (pause 1 sec)");
    }
  } else {
    if (currentTime - buzzerLastChange >= BUZZER_OFF_TIME) {
      digitalWrite(ALARM_PIN_33, HIGH);
      buzzerCurrentState = true;
      buzzerLastChange = currentTime;
      Serial.println("Buzzer: ON (2 sec)");
    }
  }
}

// =====================================================
// ЛОГИКА КОНТРОЛЯ ТЕМПЕРАТУРЫ
// =====================================================
void checkProcessLimits() {
  if (!processStarted || sensorErrorActive) {
    stopBuzzerCycle();
    return;
  }
  
  float threshold = myTmpMax + tempDev;
  
  if (myTmpCur >= threshold) {
    if (digitalRead(VALVE_PIN_26) == LOW) {
      digitalWrite(VALVE_PIN_26, HIGH);
      Serial.println("PROCESS: Temperature exceeded! Valve CLOSED (HIGH on D26)");
      sendBotMessage("🚨 ПРЕВЫШЕНИЕ ТЕМПЕРАТУРЫ!\n🌡️ Текущая: " + String(myTmpCur, 1) + 
                     "°C\n🎯 Порог: " + String(threshold, 1) + 
                     "°C\n🚰 Клапан ЗАКРЫТ", chatID);
    }
    
    if (alarmEnabled && !buzzerCycleActive) {
      startBuzzerCycle();
      sendBotMessage("🔔 Зуммер ВКЛЮЧЕН (циклический режим)", chatID);
    }
  }
  else if (myTmpCur <= myTmpMax) {
    if (digitalRead(VALVE_PIN_26) == HIGH) {
      digitalWrite(VALVE_PIN_26, LOW);
      Serial.println("PROCESS: Temperature normalized! Valve OPEN (LOW on D26)");
      sendBotMessage("✅ ТЕМПЕРАТУРА В НОРМЕ\n🌡️ Текущая: " + String(myTmpCur, 1) + 
                     "°C\n🚰 Клапан ОТКРЫТ", chatID);
    }
    
    if (buzzerCycleActive) {
      stopBuzzerCycle();
      sendBotMessage("🔕 Зуммер ВЫКЛЮЧЕН", chatID);
    }
  }
}
