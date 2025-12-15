/*
 * InputBouton.c
 * Implémentation de la librairie InputBouton
 */

#include "InputBouton.h"

// Constructeur
InputBouton::InputBouton(uint8_t buttonPin, boolean defaultButtonState, uint8_t defaultInputMode)
{
    pin = buttonPin;
    defaultState = defaultButtonState;
    inputMode = defaultInputMode;

    // Valeurs par défaut des seuils
    debounceMs = 50;
    shortPressMaxMs = 500;
    longPressMinMs = 1000;
    multiClickTimeoutMs = 300;
    maxClickCount = 2; // Par défaut, détection jusqu'au double-clic

    // Initialisation des variables d'état
    lastButtonState = defaultState;
    buttonState = defaultState;
    pressStartTime = 0;
    releaseTime = 0;
    clickCount = 0;
    lastClickCount = 0;
    lastClickTime = 0;
    longPressTriggered = false;
}

// Initialisation de la broche
void InputBouton::begin()
{
    pinMode(pin, inputMode);
}

// Configuration des seuils de temps
void InputBouton::setDebounce(unsigned long ms)
{
    debounceMs = ms;
}
void InputBouton::setShortPressMax(unsigned long ms)
{
    shortPressMaxMs = ms;
}

void InputBouton::setLongPressMin(unsigned long ms)
{
    longPressMinMs = ms;
}

void InputBouton::setMultiClickTimeout(unsigned long ms)
{
    multiClickTimeoutMs = ms;
}

void InputBouton::setMaxClickCount(uint8_t maxClicks)
{
    maxClickCount = maxClicks;
}

// Fonction principale de mise à jour
ButtonEvent InputBouton::update()
{
    ButtonEvent event = BUTTON_NO_EVENT;

    // 0. Lecture de l'état physique du bouton
    int reading = digitalRead(pin);

    // 1. Gestion Anti-rebond et Changement d'éta
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    // 2. Détection du changement d'état
    if ((millis() - lastDebounceTime) > debounceMs)
    {
        if (reading != buttonState)
        {
            buttonState = reading;

            // 3. Gestion de l'événement PRESS (Appui)
            if (buttonState != defaultState)
            { // Bouton pressé (état différent du défaut)
                pressStartTime = millis();
                longPressTriggered = false;
                event = BUTTON_PRESSED;

                // Réinitialiser le compte de clics si le délai est dépassé
                if (millis() - lastClickTime > multiClickTimeoutMs)
                {
                    clickCount = 0;
                }
            }
            // 4. Gestion de l'événement RELEASE (Relâchement)
            else
            { // Bouton relâché
                releaseTime = millis();
                unsigned long pressDuration = releaseTime - pressStartTime;
                event = BUTTON_RELEASED;

                // Détection de l'appui COURT (potentiel multi-clic)
                if (pressDuration > debounceMs && pressDuration <= shortPressMaxMs)
                {
                    clickCount++;
                    lastClickTime = millis();
                    // Limite le compte de clics au maximum configuré
                    if (clickCount > maxClickCount)
                    {
                        clickCount = maxClickCount;
                    }
                }
                // Détection des appuis longs
                else
                {
                    clickCount = 0; // Annule tout multi-clic potentiel
                    longPressTriggered = true;
                    if (pressDuration >= longPressMinMs * 2) // TRES TRES LONG
                    {
                        event = BUTTON_VERY_VERY_LONG_PRESS;
                    }
                    else if (pressDuration >= longPressMinMs) // TRES LONG
                    {
                        event = BUTTON_VERY_LONG_PRESS;
                    }
                    else // LONG
                    {
                        event = BUTTON_LONG_PRESS;
                    }
                }
            }
        }
    }

    // 5. Détection du MULTI-CLIC ou du SIMPLE-CLIC
    // Vérifier si le délai de multi-clic est écoulé
    if (clickCount > 0 && !longPressTriggered &&
        (millis() - lastClickTime > multiClickTimeoutMs))
    {

        if (clickCount == 1)
        {
            event = BUTTON_SHORT_CLICK;
        }
        else if (clickCount >= 2)
        {
            event = BUTTON_MULTI_CLICK;
        }
        lastClickCount = clickCount;
        clickCount = 0; // Réinitialisation
    }

    // Stocke l'état pour la prochaine itération
    lastButtonState = reading;

    return event;
}

// Getters
uint8_t InputBouton::getClickCount()
{
    return lastClickCount;
}

unsigned long InputBouton::getPressDuration()
{
    if (buttonState != defaultState)
    { // Bouton actuellement pressé
        return millis() - pressStartTime;
    }
    return releaseTime - pressStartTime;
}

boolean InputBouton::isPressed()
{
    return (buttonState != defaultState);
}