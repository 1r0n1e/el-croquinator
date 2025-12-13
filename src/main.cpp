#include <Arduino.h>
#include "time.h"
#include <Adafruit_SSD1306.h> // Ecran OLED
#include <Servo.h>            // Servomoteur
#include "virtuabotixRTC.h"   // RTC module 2
#include <Preferences.h>      // Persistent memory
#ifdef ESP32
#include <WiFi.h>
#include <SPIFFS.h>
#else
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <FS.h>
#endif

// --- INCLUSIONS PERSO ---
#include "config.h" //  pins et constantes
#include "images.h" //  image de chat

// --- OBJETS GLOBAUX ---
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN); // Ecran OLED
virtuabotixRTC myRTC(DS1302_CLK_PIN, DS1302_DAT_PIN, DS1302_RST_PIN);      // RTC module 2
Servo monServomoteur;                                                      // Servomoteur
Preferences preferences;                                                   // Persistent memory

// -------------------           DECLARATION DES FONCTIONS (début)           ------------------- /
void setupSPIFFS();                                                            // (setup) Connecte la mémoire persistante
void setupWiFi();                                                              // (setup) Connecte le wifi
void getSavedSettings();                                                       // (setup) Récupère la data de la mémoire persistante
void setupScreen();                                                            // (setup) Connecte l'écran OLED
void printMessage(char *title, char *message, unsigned int displayTimeSec);    // Affiche un message sur l'écran OLED
void printImage(unsigned int displayTimeSec);                                  // Affichage d'une image au centre de l'écran
void displayScreen(unsigned int displayTimeSec);                               // Affiche l'écran de bord
void openValve(unsigned int timeOpen);                                         // Donne la nourriture
void getRtcTime();                                                             // Imprime le temps RTC dans la console
unsigned long getRtcSecondsFromMidnight();                                     // Retourne le nombre de secondes depuis minuit
void convertSecondsFromMidnightToTime(unsigned long seconds, char *outBuffer); // Convertit des secondes en heure HH:MM (fournir le buffer de sortie)
char *getWiFiTime();                                                           // Récupère la date depuis le wifi
void feedCat(boolean grossePortion);                                           // Distribue les (0) Croquinettes || (1) Croquettes
// -------------------           DECLARATION DES FONCTIONS (fin)           ------------------- /

// -------------------                INITIALISATION (début)                ------------------- /
void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; // wait until Arduino Serial Monitor opens

  // Initialisation de l'écran OLED
  setupScreen();

  // Initialize persistent storage space
  // preferences.begin("croquinator-settings", false);
  // setupSPIFFS();
  // Configuration du WiFi
  // setupWiFi();
  // Récupération de l'heure depuis le WiFi
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // NTP server config

  // Configuration de l'horloge interne
  myRTC.setDS1302Time(0, 58, 17, 4, 17, 12, 2020); // sec min h, j semaine, j mois, mois, année

  // Configuration du Servomoteur
  monServomoteur.attach(SERVO_PIN);      // Attache l'objet Servo à la broche D9 de l'Arduino
  monServomoteur.write(ANGLE_FERMETURE); // S'assure que la valve est fermée au démarrage

  // Configuration du bouton poussoir
  pinMode(BOUTON_PIN, INPUT_PULLUP); // le bouton est une entrée
  etatBouton = HIGH;                 // on initialise l'état du bouton comme "relaché"

  // Configuration de la LED
  // pinMode(LED_PIN, OUTPUT);   // la led est une sortie
  // digitalWrite(LED_PIN, LOW); // Éteint la LED}

  // Just to know which program is running
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\n"));
}
// -------------------                INITIALISATION (fin)                ------------------- /

// -------------------                BOUCLE LOOP (début)                ------------------- /
void loop()
{
  // Serial.println("Début de la Boucle principale");
  // getRtcTime();

  myRTC.updateTime();
  int h = myRTC.hours;
  int m = myRTC.minutes;

  if ((h > HEURE_DEBUT_MIAM || (h == HEURE_DEBUT_MIAM && m >= MINUTE_DEBUT_MIAM)) &&
      (h < HEURE_FIN_MIAM || (h == HEURE_FIN_MIAM && m <= MINUTE_FIN_MIAM)))
  {
    // Serial.println("Dans la plage horaire de nourrissage.");

    // fonction principale

    maintenantSec = getRtcSecondsFromMidnight(); // Vérifier l'heure
    // Serial.println(maintenantSec);
    //  Si le chat est affamé : Verifier le délai depuis le dernier
    unsigned long deltaSecondes = maintenantSec - lastFeedtimeCroquettes; // interval de temps
    if (deltaSecondes >= FEED_DELAY_CROQUETTES_SEC)                       // Si le délai de 2H est écoulé
    {
      feedCat(1); // Donner des croquettes
    }

    // Vérifier les boutons
    if (etatBouton == HIGH && digitalRead(BOUTON_PIN) == LOW) // Si on vient d'appuyer sur le bouton
    {
      etatBouton = LOW;
      Serial.println("Le bouton est appuyé");
      Serial.println("Allumage de la led");
      // digitalWrite(LED_PIN, HIGH);                                 // Allume la LED
      debutappuiBoutonMs = millis(); // Enregistre le temps de début de l'appui sur le bouton
      delay(DEBOUNCE_DELAY_MS);      // Anti-rebond
    }
    else if (etatBouton == LOW && digitalRead(BOUTON_PIN) == HIGH)
    { // On vient d'arreter d'appuyer

      // digitalWrite(LED_PIN, LOW); // Éteint la LED}
      Serial.println("Extinction de la led");
      etatBouton = HIGH;
      const unsigned long duréeappuiBoutonMs = (millis() - debutappuiBoutonMs); // Calcul du temps d'appui sur le bouton en secondes

      if (duréeappuiBoutonMs <= 1000) // Si le bouton est appuyé pendant moins de 1 seconde
      {
        Serial.println("appui court détecté");
        displayScreen(5); // Afficher les dernières croquettes et croquinettes servies
      }
      else // Si le bouton est appuyé pendant plus de 1 seconde
      {
        Serial.println("appui long détecté");
        feedCat(0); // Donner des croquinettes
      }
    }
  }

  // Délais de fin de boucle
  displayScreen(5); // Afficher les dernières croquettes et croquinettes servies
  delay(1000);
}
// -------------------                BOUCLE LOOP (fin)                ------------------- /

boolean detecterCroquettes()
{
  const boolean presence = digitalRead(IR_PIN);
  Serial.print("Detection des croquettes... ");
  if (presence == LOW)
  {
    Serial.println("Des croquetttes sont dans la gamelle");
    return true;
  }
  else
  {
    Serial.println("Pas de croquetttes dans la gamelle");
    return false;
  }
}

/* Fonction pour nourrir le chat
Vérifie la présence de croquettes et les distribue
@args
grossePortion : true (croquettes) | false (croquinettes)
*/
void feedCat(boolean grossePortion)
{
  Serial.println("Nourrir le chat !");
  const boolean presenceDeCroquettes = detecterCroquettes(); // Vérifier si il y a des croquettes

  // CAS n°1 - Il y a déja des croquettes
  if (presenceDeCroquettes == true)
  {
    if (grossePortion == true)
    { // Croquettes
      Serial.println("Distribution des croquettes reportée");
      lastFeedtimeCroquettes += 30 * 1000 * 60; // Attendre 1 heure pour vérifier à nouveau si Gazou a mangé avant de lui donner à nouveau
      printMessage("No gazou", "Gazou est absent, distribution des croquettes reportée..", 2);
    }
    else
    { // Croquinettes
      Serial.println("Pas de croquinettes pour les chats qui ne mangent pas");
      printMessage("No way", "Il y a déjà des croquettes dans la gamelles !", 2);
    }
  }
  // Fin du CAS n°1 - Il y a déja des croquettes

  // CAS n°2 - Il n'y a pas de croquettes
  else
  {
    Serial.print("Distribution ");
    printImage(1); // Afficher l'image du chat
    // Bonus: Prévenir le chat avec un son ou une led

    // CAS n°1 - Croquettes
    if (grossePortion == true)
    {
      Serial.println(" des croquettes.");
      openValve(croquettes);                  // Nourrir le chat avec une portion complète
      lastFeedtimeCroquettes = maintenantSec; // Met à jour le dernier temps de nourrissage
      // preferences.putULong("lastFeedtimecroquinettes", lastFeedtimecroquinettes); // Sauvegarder le temps de nourrissage dans la mémoire persistante
      printMessage("Miam", "El Gazou a eu sa dose", 2);
      Serial.println("El Gazou a eu sa dose");
    }

    // CAS n°2 - Croquinettes
    else
    {
      Serial.println(" des croquinettes.");
      // Vérifier que le delay des croquinettes est dépassé
      unsigned long deltaSecondes = maintenantSec - lastFeedtimecroquinettes; // interval de temps
      if (deltaSecondes >= FEED_DELAY_CROQUINETTES_SEC)                       // Si le délai de 30 min est écoulé
      {
        Serial.println("Délai écoulé, on peut donner une gourmandise/crpquinette");
        openValve(croquinettes);                  // Nourrir le chat avec quelques croquettes
        lastFeedtimecroquinettes = maintenantSec; // Met à jour le dernier temps de gourmandise
        // preferences.putULong("lastFeedtimecroquinettes", lastFeedtimecroquinettes); // Sauvegarder le temps de nourrissage dans la mémoire persistante
        Serial.println("El gazou est servi !");
        printMessage("Miaou", "El Gazou est servi !", 2);
      }
      else
      {
        Serial.println("El gazou a deja eu sa gourmandise.");
        char message[56];                                                       // Nombre de caractères max pour le message
        const unsigned int deltaMinutes = deltaSecondes / 60;                   // conversion en minutes
        sprintf(message, "Dernières Croquinettes il y a %d min", deltaMinutes); // Prépare le message à afficher
        printMessage("No way", message, 2);
      }
    }
  }
  // Fin du CAS n°2 - Il n'y a pas de croquettes
};

// -------------------                FONCTIONS                ------------------- /
// Initialize SPIFFS (mémoire persistante)
void setupSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}
void getSavedSettings()
{
  // lastFeedtime = preferences.getUShort("feedtime", 0); // 0 par défaut
}
// Initialize WiFi
void setupWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
}

// Initialisation de l'écran OLED
void setupScreen()
{
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADRESS))
  {
    Serial.println(F("Erreur de communication avec le chipset SSD1306… arrêt du programme."));
    while (1)
      ; // Arrêt du programme (boucle infinie)
  }
  else
  {
    Serial.println(F("Initialisation du contrôleur SSD1306 réussi."));
  }
  printImage(1);
}
void printMessage(char *title, char *message, unsigned int displayTimeSec)
{
  oled.clearDisplay();      // Vidange du buffer de l'écran OLED
  oled.setTextColor(WHITE); // Couleur "blanche" (ou colorée, si votre afficheur monochrome est bleu, jaune, ou bleu/jaune)

  oled.setCursor(0, 1); // Positionnement du curseur dans l'angle haut-gauche, avec décalage de 1 pixel vers le bas
  oled.setTextSize(2);  // Sélection de l'échelle 1:1
  oled.println(title);
  oled.println();
  oled.setTextSize(1); // Sélection de l'échelle 1:1
  oled.println(message);

  oled.display();
  delay(displayTimeSec * 1000);
  oled.clearDisplay();
}
// Affichage d'une image au centre de l'écran
void printImage(unsigned int displayTimeSec)
{
  oled.clearDisplay();
  oled.drawBitmap(
      (oled.width() - IMAGE_WIDTH) / 2,   // Position de l'extrême "gauche" de l'image (pour centrage écran, ici)
      (oled.height() - IMAGE_HEIGHT) / 2, // Position de l'extrême "haute" de l'image (pour centrage écran, ici)
      IMAGE_CHAT,
      IMAGE_WIDTH,
      IMAGE_HEIGHT,
      WHITE); // "couleur" de l'image

  oled.display();

  delay(displayTimeSec * 1000);
  oled.clearDisplay();
}

void convertSecondsFromMidnightToTime(unsigned long seconds, char *outBuffer)
{
  // --- CONVERSION DE SEC EN HH:MM ---
  // On s'assure que la valeur reste dans les limites de 24h (86400 secondes)

  long totalSec = seconds % 86400;
  if (totalSec < 0)
    totalSec += 86400; // Gestion si le temps est négatif

  int h = totalSec / 3600;
  int m = (totalSec % 3600) / 60;

  snprintf(outBuffer, 6, "%02d:%02d", h, m);
}

void displayScreen(unsigned int displayTimeSec)
{
  const byte GRID_LINE_0 = 1;
  const byte GRID_LINE_1 = 20;
  const byte GRID_LINE_2 = 35;
  const byte GRID_LINE_3 = 50;

  const byte GRID_COL_0 = 0;
  const byte GRID_COL_1 = 80;
  const byte GRID_COL_2 = 95;

  char timeBuffer[9];  // Espace pour "HH:MM:SS\0"
  char dateBuffer[11]; // Espace pour "DD/MM/YYYY\0"
  // Formatage de l'heure et de la date dans les buffers de char
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d",
           myRTC.hours, myRTC.minutes);
  snprintf(dateBuffer, sizeof(dateBuffer), "%02d/%02d/%04d",
           myRTC.dayofmonth, myRTC.month, myRTC.year);

  // Convertir en heure
  char heureCroquettes[6];
  char heureCroquinettes[6];
  char heureProchaineCroquettes[6];
  const int prochainCroqSec = lastFeedtimeCroquettes + FEED_DELAY_CROQUETTES_SEC;
  convertSecondsFromMidnightToTime(lastFeedtimeCroquettes, heureCroquettes);
  convertSecondsFromMidnightToTime(lastFeedtimecroquinettes, heureCroquinettes);
  convertSecondsFromMidnightToTime(prochainCroqSec, heureProchaineCroquettes);

  oled.clearDisplay();      // Vidange du buffer de l'écran OLED
  oled.setTextColor(WHITE); // Couleur "blanche" (ou colorée, si votre afficheur monochrome est bleu, jaune, ou bleu/jaune)
  oled.setTextSize(1);      // Sélection de l'échelle 1:1

  oled.setCursor(GRID_COL_0, GRID_LINE_0);
  oled.print(dateBuffer);
  oled.setCursor(GRID_COL_2, GRID_LINE_0);
  oled.print(timeBuffer);

  oled.setCursor(GRID_COL_0, GRID_LINE_1);
  oled.print("Croquettes");
  oled.setCursor(GRID_COL_1, GRID_LINE_1);
  oled.print("1");
  oled.setCursor(GRID_COL_2, GRID_LINE_1);
  oled.print(heureCroquettes);

  oled.setCursor(GRID_COL_0, GRID_LINE_2);
  oled.print("Croquinettes");
  oled.setCursor(GRID_COL_1, GRID_LINE_2);
  oled.print("2");
  oled.setCursor(GRID_COL_2, GRID_LINE_2);
  oled.print(heureCroquinettes);

  oled.setCursor(GRID_COL_0, GRID_LINE_3);
  oled.print("Prochain croq");
  oled.setCursor(GRID_COL_2, GRID_LINE_3);
  oled.print(heureProchaineCroquettes);

  oled.display();
  delay(displayTimeSec * 1000);
}

void openValve(unsigned int timeOpen)
{
  Serial.print("Ouverture de la valve ");
  Serial.print(timeOpen);
  Serial.println(" ms).");

  monServomoteur.write(ANGLE_OUVERTURE); // Ouvre
  delay(timeOpen);                       // Attend 0,5 seconde pour laisser tomber la portion
  monServomoteur.write(ANGLE_FERMETURE); // Ferme
}

void getRtcTime()
{
  myRTC.updateTime();

  Serial.print("Date / Heure: ");
  Serial.print(myRTC.dayofmonth);
  Serial.print("/");
  Serial.print(myRTC.month);
  Serial.print("/");
  Serial.print(myRTC.year);
  Serial.print(" ");
  Serial.print(myRTC.hours);
  Serial.print(":");
  Serial.print(myRTC.minutes);
  Serial.print(":");
  Serial.println(myRTC.seconds);
}
unsigned long getRtcSecondsFromMidnight()
{
  myRTC.updateTime();
  unsigned long totalSeconds = myRTC.hours * 3600 + myRTC.minutes * 60 + myRTC.seconds;
  return totalSeconds;
}

// // Get time stamp
// char* getWiFiTime()
// {
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo))
//   {
//     Serial.println("Failed to obtain time");
//   }
//   // Convert to string
//   char timeStr[19];
//   strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
//   // strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
//   return 'z';
// }