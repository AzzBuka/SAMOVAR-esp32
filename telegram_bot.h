#ifndef TELEGRAM_BOT_H
#define TELEGRAM_BOT_H

#include <Arduino.h>
#include <FastBot.h>

// =====================================================
// ФУНКЦИИ TELEGRAM БОТА
// =====================================================

// Инициализация бота
void initBot();

// Обработка входящих сообщений
void handleBotMessages(FB_msg& msg);

// Отправка сообщения с защитой от спама
void sendBotMessage(String text, String chatID);

// Отправка статуса системы
void sendStatus(String chatID);

// Отправка справки по командам
void sendInfo(String chatID);

// Отправка приветственного сообщения
void sendWelcomeMessage();

// Сохранение пользовательских настроек
void saveUserSettings();

// Загрузка пользовательских настроек
void loadUserSettings();

#endif // TELEGRAM_BOT_H
