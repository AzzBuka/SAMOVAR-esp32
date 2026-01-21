#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <Arduino.h>

// =====================================================
// ФУНКЦИИ УПРАВЛЕНИЯ ПРОЦЕССОМ
// =====================================================

// Проверка лимитов температуры (логика процесса /start)
void checkProcessLimits();

// Инициализация аппаратного таймера
void initHardwareTimer();

// Обработка завершения таймера
void handleTimerFinished();

// Прерывание таймера (вызывается каждую секунду)
void IRAM_ATTR onTimer();

// Управление циклическим зуммером
void startBuzzerCycle();
void stopBuzzerCycle();
void updateBuzzerCycle();

#endif // PROCESS_CONTROL_H
