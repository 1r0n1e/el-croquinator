/*
 * AdvancedUsage.ino
 * Exemple d'utilisation avancée de la librairie ButtonManager
 *
 * Démontre :
 * - Détection jusqu'à 4 clics
 * - Personnalisation des seuils de temps
 * - Utilisation avec différents types de boutons
 * - Gestion d'actions complexes
 *
 * Ce fichier doit être placé dans : lib/ButtonManager/examples/AdvancedUsage/AdvancedUsage.ino
 */

#include <InputBouton.h>

// --- CONFIGURATION MULTI-BOUTONS ---
#define BUTTON_TACTILE_PIN D5   // Bouton tactile TTP223
#define BUTTON_MECANIQUE_PIN D6 // Bouton mécanique avec pull-up interne

// Bouton tactile : état par défaut LOW
InputBouton buttonTactile(BUTTON_TACTILE_PIN, LOW, INPUT);

// Bouton mécanique avec pull-up : état par défaut HIGH
InputBouton buttonMecanique(BUTTON_MECANIQUE_PIN, HIGH, INPUT_PULLUP);

// Variables pour les actions
int compteurSimple = 0;
int compteurDouble = 0;
int compteurTriple = 0;
int compteurQuadruple = 0;

void setup()
{
    Serial.begin(9600);
    Serial.println("=== Exemple Avancé ButtonManager ===");
    Serial.println("Configuration multi-boutons avec détection jusqu'à 4 clics");

    // --- Configuration du bouton tactile ---
    buttonTactile.begin();
    buttonTactile.setMaxClickCount(4);       // Détection jusqu'à 4 clics
    buttonTactile.setDebounce(0);            // Pas d'anti rebond
    buttonTactile.setMultiClickTimeout(400); // Délai plus long entre clics
    buttonTactile.setLongPressMin(2000);     // Appui long plus long (2s)

    // --- Configuration du bouton mécanique ---
    buttonMecanique.begin();
    buttonMecanique.setMaxClickCount(2);   // Seulement simple et double clic
    buttonMecanique.setDebounce(100);      // Anti rebond
    buttonMecanique.setShortPressMax(300); // Seuil plus court
    buttonMecanique.setLongPressMin(800);  // Appui long plus court (0.8s)

    Serial.println();
    Serial.println("Bouton tactile (D5) : jusqu'à 4 clics");
    Serial.println("Bouton mécanique (D6) : jusqu'à 2 clics");
    Serial.println();
}

void loop()
{
    // --- Gestion du bouton tactile ---
    ButtonEvent eventTactile = buttonTactile.update();

    if (eventTactile != BUTTON_NO_EVENT)
    {
        Serial.print("[TACTILE] ");

        switch (eventTactile)
        {
        case BUTTON_SHORT_CLICK:
            compteurSimple++;
            Serial.print("Simple clic (total: ");
            Serial.print(compteurSimple);
            Serial.println(")");
            // Action : allumer une LED, par exemple
            break;

        case BUTTON_MULTI_CLICK:
        {
            uint8_t clics = buttonTactile.getClickCount();
            Serial.print(clics);
            Serial.print(" clics détectés - ");

            if (clics == 2)
            {
                compteurDouble++;
                Serial.print("Double clic (total: ");
                Serial.print(compteurDouble);
                Serial.println(")");
                // Action : changer de mode
            }
            else if (clics == 3)
            {
                compteurTriple++;
                Serial.print("Triple clic (total: ");
                Serial.print(compteurTriple);
                Serial.println(")");
                // Action : réinitialiser
            }
            else if (clics == 4)
            {
                compteurQuadruple++;
                Serial.print("Quadruple clic (total: ");
                Serial.print(compteurQuadruple);
                Serial.println(")");
                // Action : menu spécial
            }
        }
        break;

        case BUTTON_LONG_PRESS:
            Serial.print("Appui long de ");
            Serial.print(buttonTactile.getPressDuration());
            Serial.println("ms - Affichage du menu");
            // Action : arrêt d'urgence
            break;

        case BUTTON_VERY_LONG_PRESS:
            Serial.print("Appui très long de ");
            Serial.print(buttonTactile.getPressDuration());
            Serial.println("ms - Reinitialisation..");
            // Action : arrêt d'urgence
            break;

        case BUTTON_VERY_VERY_LONG_PRESS:
            Serial.print("Appui très très long de ");
            Serial.print(buttonTactile.getPressDuration());
            Serial.println("ms - Arrêt d'urgence!");
            // Action : arrêt d'urgence
            break;

        case BUTTON_PRESSED:
            Serial.println("Pressé");
            break;

        case BUTTON_RELEASED:
            Serial.print("Relâché (");
            Serial.print(buttonTactile.getPressDuration());
            Serial.println("ms)");
            break;

        default:
            break;
        }
    }

    // --- Gestion du bouton mécanique ---
    ButtonEvent eventMecanique = buttonMecanique.update();

    if (eventMecanique != BUTTON_NO_EVENT)
    {
        Serial.print("[MECANIQUE] ");

        switch (eventMecanique)
        {
        case BUTTON_SHORT_CLICK:
            Serial.println("Simple clic - Navigation suivante");
            // Action : naviguer dans un menu
            break;

        case BUTTON_MULTI_CLICK:
            Serial.println("Double clic - Navigation précédente");
            // Action : retour en arrière
            break;

        case BUTTON_LONG_PRESS:
            Serial.println("Appui long - Validation");
            // Action : valider un choix
            break;

        default:
            break;
        }
    }

    // Afficher un résumé périodiquement
    static unsigned long lastSummary = 0;
    if (millis() - lastSummary > 10000)
    { // Toutes les 10 secondes
        lastSummary = millis();
        Serial.println();
        Serial.println("--- RÉSUMÉ DES ACTIONS ---");
        Serial.print("Simple clics: ");
        Serial.println(compteurSimple);
        Serial.print("Double clics: ");
        Serial.println(compteurDouble);
        Serial.print("Triple clics: ");
        Serial.println(compteurTriple);
        Serial.print("Quadruple clics: ");
        Serial.println(compteurQuadruple);
        Serial.println();
    }
}