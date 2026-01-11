#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

#include <Arduino.h>
#include <GyverSegment.h>

// =====================================================
// ФУНКЦИИ УПРАВЛЕНИЯ ДИСПЛЕЕМ
// =====================================================

// Инициализация дисплея
void initDisplay();

// Обновление дисплея (температура + время/таймер)
void updateDisplay();

// Показать сообщение сброса
void showResetMessage();

// Показать режим AP
void showAPMode();

#endif // DISPLAY_CONTROL_H
