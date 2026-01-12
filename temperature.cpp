#include "temperature.h"
#include "config.h"
#include "telegram_bot.h"

extern GyverDS18Single ds;
extern GyverDS18Single dsBowl;
extern bool alertSent;

// =====================================================
// ИНИЦИАЛИЗАЦИЯ ДАТЧИКА
// =====================================================
void initTemperatureSensor() {
  ds.requestTemp();
  lastSuccessRead = millis();
  Serial.println("Temperature sensor initialized");
}

// =====================================================
// ИНИЦИАЛИЗАЦИЯ ДАТЧИКА BOWL
// =====================================================
void initBowlSensor() {
  dsBowl.requestTemp();
  lastBowlRead = millis();                              // ДОБАВЛЕНО
  bowlSensorError = false;                              // ДОБАВЛЕНО
  Serial.println("BOWL sensor initialized");
}

// =====================================================
// ОБРАБОТКА ОШИБКИ ДАТЧИКА
// =====================================================
void handleSensorError() {
  if (!sensorErrorActive) {
    sensorErrorActive = true;
    digitalWrite(ALARM_PIN_33, HIGH);
    digitalWrite(VALVE_PIN_26, HIGH);
    Serial.println("CRITICAL: Sensor timeout!");
    sendBotMessage("🚨 КРИТИЧЕСКАЯ ОШИБКА: Датчик DS18 не отвечает более 5 минут! СИСТЕМА ОСТАНОВЛЕНА.", chatID);
  }
}

// =====================================================
// ОБРАБОТКА ВОССТАНОВЛЕНИЯ ДАТЧИКА
// =====================================================
void handleSensorRecovery() {
  if (sensorErrorActive) {
    sensorErrorActive = false;
    Serial.println("Sensor connection restored");
    sendBotMessage("✅ Связь с датчиком восстановлена. Темп: " + String(myTmpCur, 1) + "°C", chatID);
  }
}

// =====================================================
// ПРОВЕРКА ТАЙМАУТА ДАТЧИКА
// =====================================================
void checkSensorTimeout() {
  if (millis() - lastSuccessRead >= SENSOR_TIMEOUT) {
    handleSensorError();
  }
}

// =====================================================
// ОБНОВЛЕНИЕ ТЕМПЕРАТУРЫ
// =====================================================
void updateTemperature() {
  if (ds.tick()) {
    if (ds.readTemp()) {
      float temp = ds.getTemp();
      
      if (temp >= TEMP_MIN_VALID && temp <= TEMP_MAX_VALID) {
        myTmpCur = temp;
        ds.requestTemp();
        lastSuccessRead = millis();
        
        handleSensorRecovery();
        
        if (myTmpCur < myTmpMin && !alertSent) {
          sendBotMessage("⚠️ НИЗКАЯ ТЕМПЕРАТУРА: " + String(myTmpCur, 1) + "°C", chatID);
          alertSent = true;
        } 
        else if (alertSent && myTmpCur > (myTmpMin + 0.5)) {
          alertSent = false;
        }
        
      } else {
        Serial.println("Invalid temperature reading: " + String(temp) + "°C (out of 0-100°C range)");
      }
    }
  }
}

// =====================================================
// ОБНОВЛЕНИЕ ТЕМПЕРАТУРЫ BOWL
// =====================================================
void updateBowlTemperature() {
  // Проверка таймаута датчика BOWL (30 секунд)         // ДОБАВЛЕНО
  if (millis() - lastBowlRead > 30000) {                 // ДОБАВЛЕНО
    if (!bowlSensorError) {                              // ДОБАВЛЕНО
      bowlSensorError = true;                            // ДОБАВЛЕНО
      Serial.println("BOWL sensor timeout - sensor error!");  // ДОБАВЛЕНО
    }                                                    // ДОБАВЛЕНО
  }                                                      // ДОБАВЛЕНО
  
  if (dsBowl.tick()) {
    if (dsBowl.readTemp()) {
      float temp = dsBowl.getTemp();
      
      if (temp >= TEMP_MIN_VALID && temp <= TEMP_MAX_VALID) {
        bowlTmpCur = temp;
        dsBowl.requestTemp();
        lastBowlRead = millis();                         // ДОБАВЛЕНО
        
        // Восстановление после ошибки                   // ДОБАВЛЕНО
        if (bowlSensorError) {                           // ДОБАВЛЕНО
          bowlSensorError = false;                       // ДОБАВЛЕНО
          Serial.println("BOWL sensor restored: " + String(bowlTmpCur, 1) + "°C");  // ДОБАВЛЕНО
        } else {                                         // ДОБАВЛЕНО
          Serial.println("BOWL Temp: " + String(bowlTmpCur, 1) + "°C");
        }                                                // ДОБАВЛЕНО
      } else {
        Serial.println("Invalid BOWL temperature: " + String(temp) + "°C");
      }
    }
  }
}
