#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <Arduino.h>
#include <WebServer.h>

// =====================================================
// ФУНКЦИИ ВЕБ-ПОРТАЛА
// =====================================================

// Запуск режима точки доступа (AP mode)
void startAPMode();

// HTML страницы
extern const char* htmlConfig;
extern const char* htmlSuccess;

#endif // WEB_PORTAL_H
