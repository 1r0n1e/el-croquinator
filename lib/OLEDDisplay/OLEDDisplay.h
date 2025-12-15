/*
 * OLEDDisplay.h
 * Librairie complète pour gérer les écrans OLED SSD1306 (128x64 ou 128x32)
 * Basée sur Adafruit_SSD1306 avec fonctionnalités étendues
 */

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// Alignements de texte
enum TextAlign
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

// Positions verticales prédéfinies
enum VerticalPosition
{
    TOP,
    MIDDLE,
    BOTTOM
};

class OLEDDisplay
{
private:
    Adafruit_SSD1306 *display;

    // Configuration de l'écran
    uint8_t screenWidth;
    uint8_t screenHeight;
    uint8_t i2cAddress;

    // Gestion du timing d'affichage
    unsigned long displayTimerStart;
    unsigned long displayDuration;
    bool autoRefresh;

    // État actuel
    bool isDisplaying;

    // Méthodes privées utilitaires
    int16_t getAlignedX(const char *text, TextAlign align, uint8_t textSize);
    int16_t getAlignedX(String text, TextAlign align, uint8_t textSize);
    int16_t getVerticalY(VerticalPosition pos, uint8_t textSize);
    void wrapText(const char *text, uint8_t textSize, uint8_t maxWidth);

public:
    // Constructeur
    OLEDDisplay(uint8_t width = 128, uint8_t height = 64, uint8_t address = 0x3C);

    // Initialisation
    bool begin(int8_t resetPin = -1);

    // Configuration
    void setAutoRefresh(bool enabled);
    void setBrightness(uint8_t brightness); // 0-255

    // Gestion de l'affichage temporisé
    void startTimer(unsigned int seconds);
    void stopTimer();
    bool isTimerActive();
    void update(); // À appeler dans loop() pour gérer le timer

    // Effacement
    void clear();
    void clearAndDisplay();

    // Affichage de texte simple
    void printText(const char *text, uint8_t x, uint8_t y, uint8_t size = 1);
    void printText(String text, uint8_t x, uint8_t y, uint8_t size = 1);

    // Affichage de texte aligné
    void printTextAligned(const char *text, TextAlign align, uint8_t y, uint8_t size = 1);
    void printTextAligned(String text, TextAlign align, uint8_t y, uint8_t size = 1);

    // Affichage de texte centré (vertical et horizontal)
    void printTextCentered(const char *text, uint8_t size = 2);
    void printTextCentered(String text, uint8_t size = 2);

    // Affichage de message avec titre
    void printMessage(const char *title, const char *message, unsigned int displayTimeSec = 3);
    void printMessage(String title, String message, unsigned int displayTimeSec = 3);

    // Affichage de texte long avec défilement automatique (wrapping)
    void printLongText(const char *text, uint8_t size = 1, unsigned int displayTimeSec = 5);
    void printLongText(String text, uint8_t size = 1, unsigned int displayTimeSec = 5);

    // Affichage d'image/bitmap
    void printImage(const uint8_t *bitmap, uint8_t width, uint8_t height,
                    uint8_t x, uint8_t y);
    void printImageCentered(const uint8_t *bitmap, uint8_t width, uint8_t height);

    // Barre de progression
    void drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                         uint8_t progress, bool showPercentage = true);
    void drawProgressBarBottom(uint8_t progress, bool showPercentage = true);

    // Affichage de l'heure
    void printTime(uint8_t hours, uint8_t minutes,
                   TextAlign align = ALIGN_CENTER, uint8_t y = 0, uint8_t size = 1);
    void printDate(uint8_t day, uint8_t month, uint16_t year,
                   TextAlign align = ALIGN_CENTER, uint8_t y = 0, uint8_t size = 1);

    // Affichage de valeur numérique avec label
    void printValue(const char *label, float value, uint8_t decimals = 1,
                    const char *unit = "", uint8_t y = 0, uint8_t size = 1);
    void printValue(String label, float value, uint8_t decimals = 1,
                    String unit = "", uint8_t y = 0, uint8_t size = 1);

    // Indicateurs visuels
    void drawBattery(uint8_t x, uint8_t y, uint8_t percentage);
    void drawWifiSignal(uint8_t x, uint8_t y, uint8_t strength); // 0-4

    // Combinaisons prédéfinies
    void printImageWithProgress(const uint8_t *bitmap, uint8_t imgWidth, uint8_t imgHeight,
                                uint8_t progress);
    void printImageWithText(const uint8_t *bitmap, uint8_t imgWidth, uint8_t imgHeight,
                            const char *text, uint8_t textSize = 1);

    // Accès direct à l'objet Adafruit_SSD1306 pour fonctions avancées
    Adafruit_SSD1306 *getDisplay();

    // Refresh manuel
    void refresh();
};

#endif // OLED_DISPLAY_H