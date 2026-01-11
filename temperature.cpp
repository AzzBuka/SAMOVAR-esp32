#include "temperature.h"
#include "config.h"
#include "telegram_bot.h"

extern GyverDS18Single ds;
extern GyverDS18Single dsBowl;                         // ДОБАВЛЕНО
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
// ИНИЦИАЛИЗАЦИЯ ДАТЧИКА BOWL                          // ДОБАВЛЕНО
// =====================================================
void initBowlSensor() {                                 // ДОБАВЛЕНО
  dsBowl.requestTemp();                                 // ДОБАВЛЕНО
  Serial.println("BOWL sensor initialized");            // ДОБАВЛЕНО
}                                                        // ДОБАВЛЕНО

// =====================================================
// ОБРАБОТКА ОШИБКИ ДАТЧИКА
// =====================================================
void handleSensorError() {
  if (!sensorErrorActive) {
    sensorErrorActive = true;
    digitalWrite(ALARM_PIN_33, HIGH);  // Включить зуммер
    digitalWrite(VALVE_PIN_26, HIGH);  // Закрыть клапан
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
      
      // Проверка валидности температуры (0-100°C)
      if (temp >= TEMP_MIN_VALID && temp <= TEMP_MAX_VALID) {
        myTmpCur = temp;
        ds.requestTemp();
        lastSuccessRead = millis();
        
        // Проверка восстановления после ошибки
        handleSensorRecovery();
        
        // Проверка низкой температуры (независимо от процесса)
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
// ОБНОВЛЕНИЕ ТЕМПЕРАТУРЫ BOWL                         // ДОБАВЛЕНО
// =====================================================
void updateBowlTemperature() {                          // ДОБАВЛЕНО
  if (dsBowl.tick()) {                                  // ДОБАВЛЕНО
    if (dsBowl.readTemp()) {                            // ДОБАВЛЕНО
      float temp = dsBowl.getTemp();                    // ДОБАВЛЕНО
      
      // Проверка валидности температуры (0-100°C)    // ДОБАВЛЕНО
      if (temp >= TEMP_MIN_VALID && temp <= TEMP_MAX_VALID) {  // ДОБАВЛЕНО
        bowlTmpCur = temp;                              // ДОБАВЛЕНО
        dsBowl.requestTemp();                           // ДОБАВЛЕНО
        Serial.println("BOWL Temp: " + String(bowlTmpCur, 1) + "°C");  // ДОБАВЛЕНО
      } else {                                          // ДОБАВЛЕНО
        Serial.println("Invalid BOWL temperature: " + String(temp) + "°C");  // ДОБАВЛЕНО
      }                                                 // ДОБАВЛЕНО
    }                                                   // ДОБАВЛЕНО
  }                                                     // ДОБАВЛЕНО
}                                                       // ДОБАВЛЕНО
