/*
 * =====================================================
 * SAMOVAR V2 - ESP32 Temperature Control System
 * =====================================================
 * 
 * Автоматизированная система контроля температуры
 * с управлением через Telegram бот
 * 
 * Автор: [Your Name]
 * Версия: 2.0
 * Дата: 2025
 * GitHub: https://github.com/your-username/samovar-esp32
 * 
 * =====================================================
 */

// =====================================================
// ПОДКЛЮЧЕНИЕ БИБЛИОТЕК
// =====================================================
#include <WiFi.h>
#include <WebServer.h>
#include <GyverDS18.h>
#include <FastBot.h>
#include <Preferences.h>
#include "time.h"
#include <ESPping.h> 
#include <GyverSegment.h>

// =====================================================
// ПОДКЛЮЧЕНИЕ МОДУЛЕЙ
// =====================================================
#include "config.h"
#include "wifi_manager.h"
#include "telegram_bot.h"
#include "display_control.h"
#include "temperature.h"
#include "process_control.h"
#include "web_portal.h"

// =====================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// =====================================================
GyverDS18Single ds(SENSOR_PIN);
GyverDS18Single dsBowl(SENSOR_BOWL_PIN);
FastBot* bot = nullptr;
Disp7219<1> disp(DISP_DIN, DISP_CLK, DISP_CS); 
Preferences prefs;
WebServer server(80);

// =====================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ - WiFi и Telegram
// =====================================================
String wifiSSID = "";
String wifiPassword = "";
String botToken = "";
String chatID = "";
bool systemConfigured = false;
bool apMode = false;

// =====================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ - Аппаратный таймер
// =====================================================
hw_timer_t * timer = NULL;
volatile int32_t timerSeconds = 0;
volatile bool timerActive = false;
volatile bool timerFinished = false;
bool showTimerZero = false;
uint32_t timerZeroShowTime = 0;

// =====================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ - Система
// =====================================================
float myTmpMax = 50.0;     
float myTmpMin = 10.0;     
float myTmpCur = 0.0;
float bowlTmpCur = 0.0;
bool bowlSensorError = false;
uint32_t lastBowlRead = 0; 
float tempDev = 0.0;       
int waitMode = 0;          
bool alertSent = false;    

bool alarmEnabled = false;
bool valveState = false;
bool processStarted = false;
String processStartTime = "";

bool sensorErrorActive = false; 
uint32_t lastSuccessRead = 0; 
uint32_t lastWiFiCheck = 0;    
uint32_t displayTimer = 0; 
uint32_t lastBotMessage = 0;
uint32_t resetButtonPressTime = 0;
bool resetButtonPressed = false;

// =====================================================
// SETUP - ИНИЦИАЛИЗАЦИЯ
// =====================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== SAMOVAR V2 STARTING ===");
  Serial.println("Firmware version: " + String(FIRMWARE_VERSION));
  
  // --- ИНИЦИАЛИЗАЦИЯ GPIO ---
  pinMode(ALARM_PIN_33, OUTPUT);
  digitalWrite(ALARM_PIN_33, LOW);
  pinMode(VALVE_PIN_26, OUTPUT);
  digitalWrite(VALVE_PIN_26, LOW);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  
  // --- ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ ---
  initDisplay();
  
  // --- ЗАГРУЗКА НАСТРОЕК ---
  loadUserSettings();
  loadSystemConfig();
  
  // --- ПРОВЕРКА КОНФИГУРАЦИИ ---
  if(!systemConfigured || wifiSSID.length() == 0 || botToken.length() == 0 || chatID.length() == 0) {
    Serial.println("System not configured - starting AP mode");
    startAPMode();
  } else {
    // --- ПОДКЛЮЧЕНИЕ К WiFi ---
    if(connectToWiFi()) {
      // Синхронизация времени
      configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
      Serial.println("Waiting for time sync...");
      delay(2000);
      
      // Инициализация Telegram бота
      bot = new FastBot(botToken);
      initBot();
      
      // Отправка приветствия
      delay(1000);
      sendWelcomeMessage();
    } else {
      Serial.println("Failed to connect - starting AP mode");
      startAPMode();
    }
  }
  
  // --- ИНИЦИАЛИЗАЦИЯ ДАТЧИКА ТЕМПЕРАТУРЫ ---
  initTemperatureSensor();
  initBowlSensor();
  
  // --- ИНИЦИАЛИЗАЦИЯ АППАРАТНОГО ТАЙМЕРА ---
  initHardwareTimer();
  
  Serial.println("=== SETUP COMPLETE ===\n");
}

// =====================================================
// LOOP - ГЛАВНЫЙ ЦИКЛ
// =====================================================
void loop() {
  // --- ПРОВЕРКА КНОПКИ СБРОСА ---
  checkResetButton();
  
  // --- РЕЖИМ AP (настройка WiFi) ---
  if(apMode) {
    server.handleClient();
    if (millis() - displayTimer >= DISPLAY_UPDATE_INTERVAL) {
      displayTimer = millis();
      updateDisplay();
    }
    return; // Пропускаем остальную логику в режиме AP
  }
  
  // --- ПРОВЕРКА WiFi СОЕДИНЕНИЯ ---
  if (millis() - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
    lastWiFiCheck = millis();
    checkWiFiIntegrity();
  }
  
  // --- ОБРАБОТКА TELEGRAM БОТА ---
  if(bot != nullptr) {
    bot->tick();
  }
  
  // --- ОБНОВЛЕНИЕ ДИСПЛЕЯ ---
  if (millis() - displayTimer >= DISPLAY_UPDATE_INTERVAL) {
    displayTimer = millis();
    updateDisplay();
  }
  
  // --- ОБРАБОТКА ЗАВЕРШЕНИЯ ТАЙМЕРА ---
  handleTimerFinished();
  
  // --- ПРОВЕРКА ТАЙМАУТА ДАТЧИКА ---
  checkSensorTimeout();
  
  // --- ОБНОВЛЕНИЕ ТЕМПЕРАТУРЫ ---
  updateTemperature();
  updateBowlTemperature();
  
  // --- ПРОВЕРКА ЛИМИТОВ ПРОЦЕССА ---
  checkProcessLimits();
}
