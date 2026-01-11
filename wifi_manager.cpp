#include "wifi_manager.h"
#include "config.h"
#include "display_control.h"

extern Preferences prefs;
extern uint32_t resetButtonPressTime;
extern bool resetButtonPressed;

// =====================================================
// ЗАГРУЗКА КОНФИГУРАЦИИ
// =====================================================
void loadSystemConfig() {
  prefs.begin("samovar", true);
  wifiSSID = prefs.getString("wifiSSID", "");
  wifiPassword = prefs.getString("wifiPass", "");
  botToken = prefs.getString("botToken", "");
  chatID = prefs.getString("chatID", "");
  systemConfigured = prefs.getBool("configured", false);
  prefs.end();
  
  Serial.println("System Config loaded:");
  Serial.println("  WiFi SSID: " + String(wifiSSID.length() > 0 ? wifiSSID : "(not set)"));
  Serial.println("  Bot Token: " + String(botToken.length() > 0 ? "***SET***" : "(not set)"));
  Serial.println("  Chat ID: " + String(chatID.length() > 0 ? chatID : "(not set)"));
  Serial.println("  Configured: " + String(systemConfigured ? "YES" : "NO"));
}

// =====================================================
// СОХРАНЕНИЕ КОНФИГУРАЦИИ
// =====================================================
void saveSystemConfig(String ssid, String password, String token, String chat) {
  prefs.begin("samovar", false);
  prefs.putString("wifiSSID", ssid);
  prefs.putString("wifiPass", password);
  prefs.putString("botToken", token);
  prefs.putString("chatID", chat);
  prefs.putBool("configured", true);
  prefs.end();
  
  Serial.println("System Config saved!");
}

// =====================================================
// СБРОС КОНФИГУРАЦИИ
// =====================================================
void resetSystemConfig() {
  prefs.begin("samovar", false);
  prefs.clear();
  prefs.end();
  
  Serial.println("System Config RESET!");
  Serial.println("Restarting in AP mode...");
  delay(1000);
  ESP.restart();
}

// =====================================================
// ПОДКЛЮЧЕНИЕ К WiFi
// =====================================================
bool connectToWiFi() {
  if(wifiSSID.length() == 0) {
    Serial.println("No WiFi credentials saved");
    return false;
  }
  
  Serial.println("Connecting to WiFi: " + wifiSSID);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  uint32_t startTime = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
    return true;
  } else {
    Serial.println("WiFi Connection FAILED!");
    return false;
  }
}

// =====================================================
// ПРОВЕРКА СТАБИЛЬНОСТИ WiFi
// =====================================================
void checkWiFiIntegrity() {
  if(apMode) return;
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  } else if (!Ping.ping(WiFi.gatewayIP(), 1)) {
    Serial.println("Gateway unreachable, reconnecting...");
    WiFi.disconnect();
    delay(100);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  }
}

// =====================================================
// ПРОВЕРКА КНОПКИ СБРОСА
// =====================================================
void checkResetButton() {
  if(digitalRead(RESET_BUTTON_PIN) == LOW) {
    if(!resetButtonPressed) {
      resetButtonPressed = true;
      resetButtonPressTime = millis();
      Serial.println("Reset button pressed...");
    } else {
      if(millis() - resetButtonPressTime >= RESET_BUTTON_HOLD_TIME) {
        Serial.println("RESET TRIGGERED!");
        showResetMessage();  // Функция из display_control.cpp
        resetSystemConfig();
      }
    }
  } else {
    if(resetButtonPressed) {
      Serial.println("Reset button released");
      resetButtonPressed = false;
    }
  }
}
