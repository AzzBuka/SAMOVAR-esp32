#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESPping.h>

// =====================================================
// ФУНКЦИИ УПРАВЛЕНИЯ WiFi
// =====================================================

// Загрузка настроек WiFi из памяти
void loadSystemConfig();

// Сохранение настроек WiFi в память
void saveSystemConfig(String ssid, String password, String token, String chat);

// Сброс всех настроек
void resetSystemConfig();

// Подключение к WiFi сети
bool connectToWiFi();

// Проверка стабильности WiFi соединения
void checkWiFiIntegrity();

// Проверка кнопки сброса (BOOT 5 сек)
void checkResetButton();

#endif // WIFI_MANAGER_H
