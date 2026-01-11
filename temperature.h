#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <Arduino.h>
#include <GyverDS18.h>

// =====================================================
// ФУНКЦИИ РАБОТЫ С ДАТЧИКОМ ТЕМПЕРАТУРЫ
// =====================================================

// Инициализация датчика DS18B20
void initTemperatureSensor();

// Чтение температуры (вызывать в loop)
void updateTemperature();

// Проверка таймаута датчика
void checkSensorTimeout();

// Обработка ошибки датчика
void handleSensorError();

// Обработка восстановления датчика
void handleSensorRecovery();

#endif // TEMPERATURE_H
