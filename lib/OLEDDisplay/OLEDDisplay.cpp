/*
 * OLEDDisplay.cpp
 * Implémentation de la librairie OLEDDisplay
 */

#include "OLEDDisplay.h"

// Constructeur
OLEDDisplay::OLEDDisplay(uint8_t width, uint8_t height, uint8_t address)
{
    screenWidth = width;
    screenHeight = height;
    i2cAddress = address;

    display = new Adafruit_SSD1306(screenWidth, screenHeight, &Wire, -1);

    displayTimerStart = 0;
    displayDuration = 0;
    autoRefresh = true;
    isDisplaying = false;
}

// Initialisation
bool OLEDDisplay::begin(int8_t resetPin)
{
    if (!display->begin(SSD1306_SWITCHCAPVCC, i2cAddress))
    {
        return false;
    }
    display->clearDisplay();
    display->setTextColor(WHITE);
    display->display();
    return true;
}

// Configuration
void OLEDDisplay::setAutoRefresh(bool enabled)
{
    autoRefresh = enabled;
}

void OLEDDisplay::setBrightness(uint8_t brightness)
{
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(brightness);
}

// Gestion du timer
void OLEDDisplay::startTimer(unsigned int seconds)
{
    displayDuration = seconds * 1000UL;
    displayTimerStart = millis();
    isDisplaying = true;
}

void OLEDDisplay::stopTimer()
{
    isDisplaying = false;
    displayDuration = 0;
}

bool OLEDDisplay::isTimerActive()
{
    return isDisplaying && (millis() - displayTimerStart < displayDuration);
}

void OLEDDisplay::update()
{
    if (isDisplaying && (millis() - displayTimerStart >= displayDuration))
    {
        clear();
        display->display();
        isDisplaying = false;
    }
}

// Effacement
void OLEDDisplay::clear()
{
    display->clearDisplay();
}

void OLEDDisplay::clearAndDisplay()
{
    display->clearDisplay();
    display->display();
}

// Affichage de texte simple
void OLEDDisplay::printText(const char *text, uint8_t x, uint8_t y, uint8_t size)
{
    display->setTextSize(size);
    display->setCursor(x, y);
    display->print(text);
    if (autoRefresh)
        display->display();
}

void OLEDDisplay::printText(String text, uint8_t x, uint8_t y, uint8_t size)
{
    printText(text.c_str(), x, y, size);
}

// Calcul de position alignée X
int16_t OLEDDisplay::getAlignedX(const char *text, TextAlign align, uint8_t textSize)
{
    int16_t x1, y1;
    uint16_t w, h;
    display->setTextSize(textSize);
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    switch (align)
    {
    case ALIGN_LEFT:
        return 0;
    case ALIGN_CENTER:
        return (screenWidth - w) / 2;
    case ALIGN_RIGHT:
        return screenWidth - w;
    default:
        return 0;
    }
}

int16_t OLEDDisplay::getAlignedX(String text, TextAlign align, uint8_t textSize)
{
    return getAlignedX(text.c_str(), align, textSize);
}

// Affichage de texte aligné
void OLEDDisplay::printTextAligned(const char *text, TextAlign align, uint8_t y, uint8_t size)
{
    int16_t x = getAlignedX(text, align, size);
    printText(text, x, y, size);
}

void OLEDDisplay::printTextAligned(String text, TextAlign align, uint8_t y, uint8_t size)
{
    printTextAligned(text.c_str(), align, y, size);
}

// Affichage de texte centré
void OLEDDisplay::printTextCentered(const char *text, uint8_t size)
{
    int16_t x1, y1;
    uint16_t w, h;
    display->setTextSize(size);
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    int16_t x = (screenWidth - w) / 2;
    int16_t y = (screenHeight - h) / 2;

    printText(text, x, y, size);
}

void OLEDDisplay::printTextCentered(String text, uint8_t size)
{
    printTextCentered(text.c_str(), size);
}

// Affichage de message avec titre
void OLEDDisplay::printMessage(const char *title, const char *message, unsigned int displayTimeSec)
{
    clear();

    printTextAligned(title, ALIGN_CENTER, 0, 2);
    printTextAligned(message, ALIGN_CENTER, 20, 1);

    display->display();
    startTimer(displayTimeSec);
}

void OLEDDisplay::printMessage(String title, String message, unsigned int displayTimeSec)
{
    printMessage(title.c_str(), message.c_str(), displayTimeSec);
}

// Affichage de texte long
void OLEDDisplay::printLongText(const char *text, uint8_t size, unsigned int displayTimeSec)
{
    clear();
    display->setTextSize(size);
    display->setCursor(0, 0);
    display->print(text);
    display->display();
    startTimer(displayTimeSec);
}

void OLEDDisplay::printLongText(String text, uint8_t size, unsigned int displayTimeSec)
{
    printLongText(text.c_str(), size, displayTimeSec);
}

// Affichage d'image
void OLEDDisplay::printImage(const uint8_t *bitmap, uint8_t width, uint8_t height,
                             uint8_t x, uint8_t y)
{

    display->drawBitmap(x, y, bitmap, width, height, WHITE);
    if (autoRefresh)
        display->display();
}

void OLEDDisplay::printImageCentered(const uint8_t *bitmap, uint8_t width, uint8_t height)
{
    clear();
    uint8_t x = (screenWidth - width) / 2;
    uint8_t y = (screenHeight - height) / 2;
    printImage(bitmap, width, height, x, y);
}

// Barre de progression
void OLEDDisplay::drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                                  uint8_t progress, bool showPercentage)
{
    // Limiter le progrès entre 0 et 100
    if (progress > 100)
        progress = 100;

    // Dessiner le contour
    display->drawRect(x, y, width, height, WHITE);

    // Remplir la barre selon le progrès
    uint8_t fillWidth = ((width - 2) * progress) / 100;
    if (fillWidth > 0)
    {
        display->fillRect(x + 1, y + 1, fillWidth, height - 2, WHITE);
    }

    // Afficher le pourcentage si demandé
    if (showPercentage)
    {
        char buffer[5];
        sprintf(buffer, "%d%%", progress);

        int16_t x1, y1;
        uint16_t w, h;
        display->setTextSize(1);
        display->getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

        // Centrer le texte sur la barre
        uint8_t textX = x + (width - w) / 2;
        uint8_t textY = y + (height - h) / 2;

        // Inverser la couleur du texte pour qu'il soit visible
        display->setTextColor(BLACK, WHITE);
        display->setCursor(textX, textY);
        display->print(buffer);
        display->setTextColor(WHITE); // Restaurer la couleur
    }

    if (autoRefresh)
        display->display();
}

void OLEDDisplay::drawProgressBarBottom(uint8_t progress, bool showPercentage)
{
    drawProgressBar(0, screenHeight - 10, screenWidth, 10, progress, showPercentage);
}

// Affichage de l'heure
void OLEDDisplay::printTime(uint8_t hours, uint8_t minutes,
                            TextAlign align, uint8_t y, uint8_t size)
{
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hours, minutes);
    printTextAligned(timeStr, align, y, size);
}

void OLEDDisplay::printDate(uint8_t day, uint8_t month, uint16_t year,
                            TextAlign align, uint8_t y, uint8_t size)
{
    char dateStr[11];
    sprintf(dateStr, "%02d/%02d/%04d", day, month, year);
    printTextAligned(dateStr, align, y, size);
}

// Affichage de valeur avec label
void OLEDDisplay::printValue(const char *label, float value, uint8_t decimals,
                             const char *unit, uint8_t y, uint8_t size)
{
    char valueStr[32];
    dtostrf(value, 0, decimals, valueStr);

    display->setTextSize(size);
    display->setCursor(0, y);
    display->print(label);
    display->print(valueStr);
    display->print(unit);

    if (autoRefresh)
        display->display();
}

void OLEDDisplay::printValue(String label, float value, uint8_t decimals,
                             String unit, uint8_t y, uint8_t size)
{
    printValue(label.c_str(), value, decimals, unit.c_str(), size);
}

// Indicateur de batterie
void OLEDDisplay::drawBattery(uint8_t x, uint8_t y, uint8_t percentage)
{
    // Corps de la batterie
    display->drawRect(x, y, 20, 10, WHITE);
    // Pôle +
    display->fillRect(x + 20, y + 3, 2, 4, WHITE);

    // Remplissage selon le niveau
    uint8_t fillWidth = (16 * percentage) / 100;
    if (fillWidth > 0)
    {
        display->fillRect(x + 2, y + 2, fillWidth, 6, WHITE);
    }

    if (autoRefresh)
        display->display();
}

// Signal WiFi
void OLEDDisplay::drawWifiSignal(uint8_t x, uint8_t y, uint8_t strength)
{
    // Limiter entre 0 et 4
    if (strength > 4)
        strength = 4;

    for (uint8_t i = 0; i < strength; i++)
    {
        uint8_t barHeight = (i + 1) * 2;
        display->fillRect(x + (i * 4), y + (8 - barHeight), 3, barHeight, WHITE);
    }

    if (autoRefresh)
        display->display();
}

// Combinaisons prédéfinies
void OLEDDisplay::printImageWithProgress(const uint8_t *bitmap, uint8_t imgWidth,
                                         uint8_t imgHeight, uint8_t progress)
{
    clear();

    // Image centrée en haut
    uint8_t imgX = (screenWidth - imgWidth) / 2;
    uint8_t imgY = 5;
    printImage(bitmap, imgWidth, imgHeight, imgX, imgY);

    // Barre de progression en bas
    drawProgressBarBottom(progress, true);

    display->display();
}

void OLEDDisplay::printImageWithText(const uint8_t *bitmap, uint8_t imgWidth,
                                     uint8_t imgHeight, const char *text, uint8_t textSize)
{
    clear();

    // Image en haut centrée
    uint8_t imgX = (screenWidth - imgWidth) / 2;
    printImage(bitmap, imgWidth, imgHeight, imgX, 0);

    // Texte en bas
    uint8_t textY = imgHeight + 5;
    printTextAligned(text, ALIGN_CENTER, textY, textSize);

    display->display();
}

// Accès à l'objet Adafruit
Adafruit_SSD1306 *OLEDDisplay::getDisplay()
{
    return display;
}

// Refresh manuel
void OLEDDisplay::refresh()
{
    display->display();
}