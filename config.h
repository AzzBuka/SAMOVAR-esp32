#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =====================================================
// ВЕРСИЯ ПРОШИВКИ
// =====================================================
#define FIRMWARE_VERSION "2.0"

// =====================================================
// ПИНЫ GPIO
// =====================================================
#define ALARM_PIN_33 33         // Зуммер
#define VALVE_PIN_26 26         // Клапан
#define SENSOR_PIN 2            // DS18B20
#define SENSOR_BOWL_PIN 4       // DS18B20 BOWL (дополнительный датчик)
#define DISP_CS 5               // MAX7219 Chip Select
#define DISP_CLK 18             // MAX7219 Clock
#define DISP_DIN 19             // MAX7219 Data In
#define RESET_BUTTON_PIN 0      // Кнопка BOOT (сброс настроек)

// =====================================================
// КОНСТАНТЫ ВРЕМЕНИ
// =====================================================
const uint32_t WIFI_CHECK_INTERVAL = 30000;          // 30 сек - проверка WiFi
const uint32_t SENSOR_TIMEOUT = 300000;              // 5 мин - таймаут датчика
const uint32_t DISPLAY_UPDATE_INTERVAL = 500;        // 0.5 сек - обновление дисплея
const uint32_t BOT_MESSAGE_INTERVAL = 2000;          // 2 сек - минимальный интервал между сообщениями бота
const uint32_t RESET_BUTTON_HOLD_TIME = 5000;       // 5 сек - удержание кнопки для сброса
const uint32_t TIMER_ZERO_DISPLAY_DURATION = 1000;  // 1 сек - показ нуля таймера

// =====================================================
// ТЕМПЕРАТУРНЫЕ КОНСТАНТЫ
// =====================================================
const float TEMP_MIN_VALID = 0.0;      // Минимальная валидная температура
const float TEMP_MAX_VALID = 100.0;    // Максимальная валидная температура

// =====================================================
// НАСТРОЙКИ NTP
// =====================================================
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 3 * 3600;  // GMT+3
const int DAYLIGHT_OFFSET_SEC = 0;

// =====================================================
// НАСТРОЙКИ AP MODE
// =====================================================
const char* AP_PASSWORD = "12345678";   // Пароль точки доступа

// =====================================================
// НАСТРОЙКИ ДИСПЛЕЯ
// =====================================================
const uint8_t DISPLAY_BRIGHTNESS = 5;   // Яркость дисплея (0-15)

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

#endif // CONFIG_H
