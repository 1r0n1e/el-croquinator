/*
 * InputBouton.h
 * Librairie pour gérer les boutons avec détection d'appuis courts, longs, et multiples clics
 * Compatible avec tous types de boutons (tactiles, mécaniques, avec ou sans pull-up)
 *
 * 15/12/25 v0
 */

#ifndef INPUT_BOUTON_H
#define INPUT_BOUTON_H

#include <Arduino.h>

// Types d'événements détectables
enum ButtonEvent
{
    BUTTON_NO_EVENT = 0,
    BUTTON_PRESSED,              // Bouton vient d'être pressé
    BUTTON_RELEASED,             // Bouton vient d'être relâché
    BUTTON_SHORT_CLICK,          // Appui court (simple clic)
    BUTTON_LONG_PRESS,           // Appui long détecté
    BUTTON_VERY_LONG_PRESS,      // Appui très long détecté
    BUTTON_VERY_VERY_LONG_PRESS, // Appui très très long détecté
    BUTTON_MULTI_CLICK           // Clics multiples (double, triple, etc.)
};

class InputBouton
{
private:
    // Configuration du bouton
    uint8_t pin;          // Broche GPIO du bouton
    boolean defaultState; // État par défaut (HIGH ou LOW)
    uint8_t inputMode;    // Pin mode (INPUT ou INPUT_PULLUP)

    // Seuils de temps configurables
    unsigned long debounceMs;
    unsigned long shortPressMaxMs;
    unsigned long longPressMinMs;
    unsigned long multiClickTimeoutMs;
    uint8_t maxClickCount; // Nombre maximum de clics à détecter

    // Variables d'état
    boolean lastButtonState;
    boolean buttonState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    unsigned long releaseTime;
    uint8_t clickCount;
    uint8_t lastClickCount;
    unsigned long lastClickTime;

    // Drapeaux d'événements
    boolean longPressTriggered;

public:
    // Constructeur
    InputBouton(uint8_t buttonPin, boolean defaultButtonState = LOW, uint8_t defaultInputMode = INPUT_PULLUP);

    // Initialisation
    void begin();

    // Configuration des seuils de temps (en millisecondes)
    void setDebounce(unsigned long ms);
    void setShortPressMax(unsigned long ms);
    void setLongPressMin(unsigned long ms);
    void setMultiClickTimeout(unsigned long ms);
    void setMaxClickCount(uint8_t maxClicks);

    // Fonction principale à appeler dans loop()
    ButtonEvent update();

    // Getters pour obtenir des informations
    uint8_t getClickCount();          // Nombre de clics détectés
    unsigned long getPressDuration(); // Durée du dernier appui
    boolean isPressed();              // État actuel du bouton
};

#endif // INPUT_BOUTON_H
