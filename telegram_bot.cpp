#include "telegram_bot.h"
#include "config.h"
#include "process_control.h"
#include <Preferences.h>

extern FastBot* bot;
extern Preferences prefs;
extern uint32_t lastBotMessage;
extern int waitMode;

// =====================================================
// ПОЛУЧЕНИЕ ВРЕМЕНИ В СТРОКУ
// =====================================================
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "Ошибка времени";
  char buf[20];
  strftime(buf, sizeof(buf), "%H:%M:%S %d.%m.%y", &timeinfo);
  return String(buf);
}

// =====================================================
// ОТПРАВКА СООБЩЕНИЯ С ЗАЩИТОЙ ОТ СПАМА
// =====================================================
void sendBotMessage(String text, String chatID) {
  if(bot == nullptr) return;
  if (millis() - lastBotMessage >= BOT_MESSAGE_INTERVAL) {
    bot->sendMessage(text, chatID);
    lastBotMessage = millis();
    Serial.println("BOT MSG: " + text);
  }
}

// =====================================================
// ОТПРАВКА СТАТУСА
// =====================================================
void sendStatus(String cID) {
  if(bot == nullptr) return;
  
  String processStatus = processStarted ? ("✅ RUNNING\n🕐 Started: " + processStartTime) : "⏸️ STOPPED";
  String alarmStatus = alarmEnabled ? "ON" : "OFF";
  String valveStatus = digitalRead(VALVE_PIN_26) ? "HIGH" : "LOW";
  String timerStr = timerActive ? String(timerSeconds) + " сек" : "OFF";
  String sensorStatus = sensorErrorActive ? "❌ ERROR" : "✅ OK";
  String alarmPinStatus = digitalRead(ALARM_PIN_33) ? "HIGH" : "LOW";
  
  // BOWL температура или ошибка                        // ИЗМЕНЕНО
  String bowlStatus = bowlSensorError ? "sensor error" : String(bowlTmpCur, 1) + "°C";  // ИЗМЕНЕНО
  
  String s = "📊 *SYSTEM STATUS*\n"
             "----------------------------\n"
             "🎬 Process: " + processStatus + "\n"
             "----------------------------\n"
             "🕐 Time: " + getTimeString() + "\n"
             "🌡️ Temperature: " + String(myTmpCur, 1) + "°C " + sensorStatus + "\n"
             "🥣 BOWL Temp: " + bowlStatus + "\n"           // ИЗМЕНЕНО
             "----------------------------\n"
             "📈 MAX Threshold: " + String(myTmpMax, 1) + "°C\n"
             "📉 MIN Threshold: " + String(myTmpMin, 1) + "°C\n"
             "⚙️ Deviation: +" + String(tempDev, 1) + "°C\n"
             "🎯 Trigger at: " + String(myTmpMax + tempDev, 1) + "°C\n"
             "⏲️ Timer: " + timerStr + "\n"
             "----------------------------\n"
             "🔔 Alarm Enabled: " + alarmStatus + "\n"
             "📍 Alarm Pin (D33): " + alarmPinStatus + "\n"
             "🚰 Valve Pin (D26): " + valveStatus + "\n"
             "----------------------------\n"
             "📶 WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n"
             "🆙 Uptime: " + String(millis() / 60000) + " min";
  
  bot->sendMessage(s, cID);
}

// =====================================================
// ОТПРАВКА СПРАВКИ
// =====================================================
void sendInfo(String cID) {
  if(bot == nullptr) return;
  String m = "📖 *СПРАВКА ПО КОМАНДАМ:*\n\n"
             "📊 *Информация:*\n"
             "/? - Состояние системы\n"
             "/help - Эта справка\n\n"
             "🌡️ *Настройки температуры:*\n"
             "/temp_up - Установить MAX (°C)\n"
             "/temp_down - Установить MIN (°C)\n"
             "/temp_current - MAX = текущая темп.\n"
             "/temp_dev - Девиация (0-5°C)\n\n"
             "⏲️ *Таймер:*\n"
             "/set_timer - Запуск (1-9999 сек)\n\n"
             "🎮 *Управление:*\n"
             "/start - Запустить процесс контроля\n"
             "/alarm_on - Разрешить зуммер (D33)\n"
             "/alarm_off - Запретить зуммер (D33)\n"
             "/zummer - Переключить зуммер (триггер)\n"
             "/valve_on - Клапан HIGH (D26)\n"
             "/valve_off - Клапан LOW (D26)";
  bot->sendMessage(m, cID);
}

// =====================================================
// ПРИВЕТСТВЕННОЕ СООБЩЕНИЕ
// =====================================================
void sendWelcomeMessage() {
  if(bot == nullptr) return;
  String h = "🚀 *SAMOVAR V2 ЗАПУЩЕН*\n"
             "📅 " + getTimeString() + "\n"
             "🌐 IP: " + WiFi.localIP().toString() + "\n"
             "📶 RSSI: " + String(WiFi.RSSI()) + " dBm\n"
             "🌡️ Температура: " + String(myTmpCur, 1) + "°C\n\n"
             "Используйте /help для справки";
  bot->sendMessage(h, chatID);
}

// =====================================================
// ОБРАБОТКА СООБЩЕНИЙ
// =====================================================
void handleBotMessages(FB_msg& msg) {
  Serial.println("MSG from " + msg.chatID + ": " + msg.text);
  
  // РЕЖИМ ОЖИДАНИЯ ВВОДА
  if (waitMode > 0) {
    if (waitMode == 4) { // Таймер
      int val = msg.text.toInt();
      if (val >= 1 && val <= 9999) {
        timerSeconds = val;
        timerActive = true;
        timerFinished = false;
        bot->sendMessage("⏲️ Таймер запущен на " + String(val) + " сек.", msg.chatID);
        waitMode = 0;
      } else {
        bot->sendMessage("❌ Ошибка! Введите число от 1 до 9999", msg.chatID);
      }
      return;
    }
    
    float val = msg.text.toFloat();
    
    if (waitMode == 3) { // Девиация
      if (val >= 0.0 && val <= 5.0) {
        tempDev = val;
        saveUserSettings();
        waitMode = 0;
        sendStatus(msg.chatID);
      } else {
        bot->sendMessage("❌ Введите значение от 0 до 5°C", msg.chatID);
      }
    } 
    else if (waitMode == 1 || waitMode == 2) { // MAX или MIN
      if (val >= 0.0 && val <= 100.0) {
        if (waitMode == 1) myTmpMax = val;
        else myTmpMin = val;
        saveUserSettings();
        waitMode = 0;
        sendStatus(msg.chatID);
      } else {
        bot->sendMessage("❌ Введите значение от 0 до 100°C", msg.chatID);
      }
    }
    return;
  }
  
  // КОМАНДЫ
  if (msg.text == "/temp_up") {
    bot->sendMessage("Введите порог MAX (0-100°C):", msg.chatID);
    waitMode = 1;
  } 
  else if (msg.text == "/temp_down") {
    bot->sendMessage("Введите порог MIN (0-100°C):", msg.chatID);
    waitMode = 2;
  }
  else if (msg.text == "/temp_current") {
    if (myTmpCur >= 0.0 && myTmpCur <= 100.0) {
      myTmpMax = myTmpCur;
      saveUserSettings();
      bot->sendMessage("✅ Порог MAX установлен на текущую температуру: " + String(myTmpMax, 1) + "°C", msg.chatID);
      sendStatus(msg.chatID);
    } else {
      bot->sendMessage("❌ Текущая температура вне допустимого диапазона (0-100°C)", msg.chatID);
    }
  }
  else if (msg.text == "/temp_dev") {
    bot->sendMessage("Введите девиацию (0-5°C):", msg.chatID);
    waitMode = 3;
  }
  else if (msg.text == "/set_timer") {
    bot->sendMessage("Введите время таймера (1-9999 сек):", msg.chatID);
    waitMode = 4;
  }
  
  // КОМАНДА /start
  else if (msg.text == "/start") {
    if (!processStarted) {
      processStarted = true;
      processStartTime = getTimeString();
      digitalWrite(VALVE_PIN_26, LOW);
      Serial.println("PROCESS STARTED at " + processStartTime);
      bot->sendMessage("🚀 ПРОЦЕСС ЗАПУЩЕН\n🕐 " + processStartTime + "\n🚰 Клапан ОТКРЫТ (LOW)", msg.chatID);
      saveUserSettings();
    } else {
      bot->sendMessage("⚠️ Процесс уже запущен с " + processStartTime, msg.chatID);
    }
  }
  
  // УПРАВЛЕНИЕ ЗУММЕРОМ
  else if (msg.text == "/alarm_on") {
    alarmEnabled = true;
    saveUserSettings();
    bot->sendMessage("🔔 Зуммер РАЗРЕШЕН (Alarm Enabled: ON)\n⚠️ Зуммер будет включаться автоматически при превышении температуры", msg.chatID);
    Serial.println("Alarm globally enabled");
  }
  else if (msg.text == "/alarm_off") {
    alarmEnabled = false;
    digitalWrite(ALARM_PIN_33, LOW);
    saveUserSettings();
    bot->sendMessage("🔕 Зуммер ЗАПРЕЩЕН (Alarm Enabled: OFF)\n📍 D33 установлен в LOW", msg.chatID);
    Serial.println("Alarm globally disabled");
  }
  
  // ТРИГГЕР ЗУММЕРА
  else if (msg.text == "/zummer") {
    if (alarmEnabled) {
      bool currentState = digitalRead(ALARM_PIN_33);
      digitalWrite(ALARM_PIN_33, !currentState);
      
      String newState = !currentState ? "HIGH (ВКЛ)" : "LOW (ВЫКЛ)";
      bot->sendMessage("🔔 Зуммер переключен\n📍 D33: " + newState, msg.chatID);
      Serial.println("Zummer toggled to: " + String(!currentState ? "HIGH" : "LOW"));
    } else {
      bot->sendMessage("⚠️ ОШИБКА: Зуммер запрещен!\nСначала выполните /alarm_on для разрешения использования зуммера.", msg.chatID);
      Serial.println("Zummer toggle DENIED: Alarm not enabled");
    }
  }
  
  // УПРАВЛЕНИЕ КЛАПАНОМ
  else if (msg.text == "/valve_on") {
    digitalWrite(VALVE_PIN_26, HIGH);
    valveState = true;
    saveUserSettings();
    bot->sendMessage("🚰 Клапан: HIGH (D26 = HIGH)", msg.chatID);
    Serial.println("Valve manually set to HIGH");
  }
  else if (msg.text == "/valve_off") {
    digitalWrite(VALVE_PIN_26, LOW);
    valveState = false;
    saveUserSettings();
    bot->sendMessage("🚰 Клапан: LOW (D26 = LOW)", msg.chatID);
    Serial.println("Valve manually set to LOW");
  }
  
  // ИНФОРМАЦИЯ
  else if (msg.text == "/?") {
    sendStatus(msg.chatID);
  }
  else if (msg.text == "/help") {
    sendInfo(msg.chatID);
  }
  else {
    bot->sendMessage("❓ Неизвестная команда. Используйте /help", msg.chatID);
  }
}

// =====================================================
// СОХРАНЕНИЕ ПОЛЬЗОВАТЕЛЬСКИХ НАСТРОЕК
// =====================================================
void saveUserSettings() {
  prefs.begin("samovar", false);
  prefs.putFloat("maxTemp", myTmpMax);
  prefs.putFloat("minTemp", myTmpMin);
  prefs.putFloat("tempDev", tempDev);
  prefs.putBool("alarmEnabled", alarmEnabled);
  prefs.putBool("valveState", valveState);
  prefs.end();
  Serial.println("User settings saved");
}

// =====================================================
// ЗАГРУЗКА ПОЛЬЗОВАТЕЛЬСКИХ НАСТРОЕК
// =====================================================
void loadUserSettings() {
  prefs.begin("samovar", true);
  myTmpMax = prefs.getFloat("maxTemp", 50.0);
  myTmpMin = prefs.getFloat("minTemp", 10.0);
  tempDev = prefs.getFloat("tempDev", 0.0);
  alarmEnabled = prefs.getBool("alarmEnabled", false);
  valveState = prefs.getBool("valveState", false);
  prefs.end();
  Serial.println("User settings loaded");
}

// =====================================================
// ИНИЦИАЛИЗАЦИЯ БОТА
// =====================================================
void initBot() {
  if(bot != nullptr) {
    bot->attach(handleBotMessages);
    Serial.println("Telegram bot initialized");
  }
}
