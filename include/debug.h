#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

/* CONFIGURATION DEBUG */
#ifdef DEBUG_MODE // Activer dans platformio.ini : build_flags = -D DEBUG_MODE
#define SERIAL_BAUD_RATE 9600
// Définition des macros
#define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#define DEBUG_PRINT(x)   \
    if (Serial)          \
    {                    \
        Serial.print(x); \
    }
#define DEBUG_PRINTLN(x)   \
    if (Serial)            \
    {                      \
        Serial.println(x); \
    }
#define DEBUG_INIT(speed)              \
    Serial.begin(speed);               \
    while (!Serial && millis() < 5000) \
        ; // Wait max 5s
#else
// Si DEBUG est désactivé, ces lignes sont effacées du code compilé
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_INIT(speed)
#define DEBUG_PRINTF(x, ...)
#endif

#endif