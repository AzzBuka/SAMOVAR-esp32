#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =====================================================
// ПИНЫ GPIO
// =====================================================
#define ALARM_PIN_33 33
#define VALVE_PIN_26 26
#define SENSOR_PIN 23
#define SENSOR_BOWL_PIN 4
#define DISP_CS 5
#define DISP_CLK 18
#define DISP_DIN 19
#define RESET_BUTTON_PIN 0

// =====================================================
// КОНСТАНТЫ ВРЕМЕНИ
// =====================================================
const uint32_t WIFI_CHECK_INTERVAL = 30000;
const uint32_t SENSOR_TIMEOUT = 300000;
const uint32_t DISPLAY_UPDATE_INTERVAL = 500;
const uint32_t BOT_MESSAGE_INTERVAL = 2000;
const uint32_t RESET_BUTTON_HOLD_TIME = 5000;
const uint32_t TIMER_ZERO_DISPLAY_DURATION = 1000;

// =====================================================
// ТЕМПЕРАТУРНЫЕ КОНСТАНТЫ
// =====================================================
const float TEMP_MIN_VALID = 0.0;
const float TEMP_MAX_VALID = 100.0;

// =====================================================
// НАСТРОЙКИ NTP И AP (ИЗМЕНЕНО: extern)
// =====================================================
extern const char* NTP_SERVER;           // ИЗМЕНЕНО: было const char*
extern const long GMT_OFFSET_SEC;
extern const int DAYLIGHT_OFFSET_SEC;
extern const char* AP_PASSWORD;          // ИЗМЕНЕНО: было const char*
extern const uint8_t DISPLAY_BRIGHTNESS;

// =====================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ (внешние объявления)
// =====================================================
extern String wifiSSID;
extern String wifiPassword;
extern String botToken;
extern String chatID;
extern bool systemConfigured;
extern bool apMode;

extern float myTmpMax;
extern float myTmpMin;
extern float myTmpCur;
extern float tempDev;
extern float bowlTmpCur;
extern bool bowlSensorError;
extern uint32_t lastBowlRead;

extern bool alarmEnabled;
extern bool valveState;
extern bool processStarted;
extern String processStartTime;

extern bool sensorErrorActive;
extern uint32_t lastSuccessRead;

extern volatile int32_t timerSeconds;
extern volatile bool timerActive;
extern volatile bool timerFinished;

extern bool showTimerZero;
extern uint32_t timerZeroShowTime;

// Переменные для циклического зуммера
extern bool buzzerCycleActive;
extern bool buzzerCurrentState;
extern uint32_t buzzerLastChange;

#endif // CONFIG_H
