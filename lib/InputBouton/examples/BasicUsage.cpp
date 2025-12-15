/*
 * BasicUsage.ino
 * Exemple d'utilisation basique de la librairie ButtonManager
 *
 * Ce fichier doit être placé dans : lib/ButtonManager/examples/BasicUsage/BasicUsage.ino
 */

#include <InputBouton.h>

// --- CONFIGURATION ---
#define BUTTON_PIN D5 // Broche connectée au bouton

// Créer une instance du gestionnaire de bouton
// Pour un bouton TTP223 (tactile) : état par défaut LOW, passe à HIGH quand pressé
InputBouton button(BUTTON_PIN, LOW, INPUT);

// Pour un bouton mécanique avec INPUT_PULLUP : état par défaut HIGH, passe à LOW quand pressé
// ButtonManager button(BUTTON_PIN, HIGH, INPUT_PULLUP);

void setup()
{
    Serial.begin(9600);
    Serial.println("=== Exemple Basic ButtonManager ===");
    Serial.println("Appuyez sur le bouton pour tester les différents types d'appuis");

    // Initialiser le bouton
    button.begin();

    // Configuration optionnelle des seuils (sinon valeurs par défaut)
    button.setDebounce(0);            // Pas d'anti rebond
    button.setShortPressMax(500);     // Appui court max : 500ms
    button.setLongPressMin(1000);     // Appui long min : 1000ms
    button.setMultiClickTimeout(300); // Délai entre clics : 300ms
    button.setMaxClickCount(2);       // Détection jusqu'au double-clic
}

void loop()
{
    // Appeler update() à chaque itération
    ButtonEvent event = button.update();

    // Traiter les événements
    switch (event)
    {
    case BUTTON_PRESSED:
        Serial.println("-> BOUTON PRESSE");
        break;

    case BUTTON_RELEASED:
        Serial.print("-> BOUTON RELACHE (Duree: ");
        Serial.print(button.getPressDuration());
        Serial.println("ms)");
        break;

    case BUTTON_SHORT_CLICK:
        Serial.println("--- APPUI COURT (SIMPLE CLIC) DETECTE ---");
        // Votre code ici pour gérer un simple clic
        break;

    case BUTTON_LONG_PRESS:
        Serial.println("--- APPUI LONG DETECTE ---");
        // Votre code ici pour gérer un appui long
        break;

    case BUTTON_MULTI_CLICK:
        Serial.print("--- MULTI-CLIC DETECTE (");
        Serial.print(button.getClickCount());
        Serial.println(" clics) ---");
        // Votre code ici pour gérer un double-clic ou plus
        break;

    case BUTTON_NO_EVENT:
        // Rien à faire
        break;
    }
}