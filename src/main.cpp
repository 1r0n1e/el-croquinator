#include <Arduino.h>
#include "time.h"
#include <Adafruit_SSD1306.h> // Ecran OLED
#include <Servo.h>            // Servomoteur
#include "virtuabotixRTC.h"   // RTC module 2
#include <Preferences.h>      // Persistent memory
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

// --- INCLUSIONS PERSO ---
#include "config.h" //  pins et constantes
#include "images.h" //  image de chat

// --- OBJETS GLOBAUX ---
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN); // Ecran OLED
virtuabotixRTC myRTC(DS1302_CLK_PIN, DS1302_DAT_PIN, DS1302_RST_PIN);      // RTC module 2
Servo monServomoteur;                                                      // Servomoteur
Preferences preferences;                                                   // Persistent memory

// -------------------           DECLARATION DES FONCTIONS (début)           ------------------- /                                                           // (setup) Connecte la mémoire persistante
void setupWiFi();                                                                       // (setup) Connecte le wifi
void getSavedSettings();                                                                // (setup) Récupère la data de la mémoire persistante
void setupScreen();                                                                     // (setup) Connecte l'écran OLED
void printMessage(const char *title, const char *message, unsigned int displayTimeSec); // Affiche un message sur l'écran OLED
void printImage(unsigned int displayTimeSec);                                           // Affichage d'une image au centre de l'écran
void displayScreen(unsigned int displayTimeSec);                                        // Affiche l'écran de bord
void openValve(unsigned int timeOpen);                                                  // Donne la nourriture
void printWiFiTime();                                                                   // Imprime la date wifi
void syncRTCFromWiFi();                                                                 // Synchronise la date RTC avec la date WiFi
void getRtcTime();                                                                      // Imprime le temps RTC dans la console
unsigned long getRtcSecondsFromMidnight();                                              // Retourne le nombre de secondes depuis minuit
void convertSecondsFromMidnightToTime(unsigned long seconds, char *outBuffer);          // Convertit des secondes en heure HH:MM (fournir le buffer de sortie)
void feedCat(boolean grossePortion);                                                    // Distribue les (0) Croquinettes || (1) Croquettes
// -------------------           DECLARATION DES FONCTIONS (fin)           ------------------- /

// -------------------                INITIALISATION (début)                ------------------- /
void setup()
{
  Serial.begin(9600);
  // while (!Serial); // wait until Arduino Serial Monitor opens
  //  Just to know which program is running
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\n"));

  // Initialisation de l'écran OLED
  setupScreen();
  // Initialize persistent storage space
  getSavedSettings();
  // Configuration du WiFi
  setupWiFi();
  // Récupération de l'heure depuis le WiFi
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // NTP server config
  printWiFiTime();
  // Syncrhonisation de l'horloge interne
  syncRTCFromWiFi();
  // myRTC.setDS1302Time(0, 58, 17, 4, 17, 12, 2020); // sec min h, j semaine, j mois, mois, année

  // Configuration du Servomoteur
  monServomoteur.attach(SERVO_PIN);      // Attache l'objet Servo à la broche D9 de l'Arduino
  monServomoteur.write(ANGLE_FERMETURE); // S'assure que la valve est fermée au démarrage

  // Configuration du bouton poussoir
  pinMode(BOUTON_PIN, INPUT_PULLUP); // le bouton est une entrée

  // displayScreen(5); // afficher l'écran de bord
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
  int s = myRTC.seconds;
  if (h == 23 && m == 59 && s == 59)
  {
    syncRTCFromWiFi();
    // Réinitialisation des compteurs
    compteurDeCroquettes = 0;
    compteurDeCroquinettes = 0;
    lastFeedtimeCroquettes = ((HEURE_DEBUT_MIAM - 2) * 60 + MINUTE_DEBUT_MIAM) * 60;
    lastFeedtimecroquinettes = 0;
  }
  // --------- PLAGE HORAIRE (début) --------- //
  if ((h > HEURE_DEBUT_MIAM || (h == HEURE_DEBUT_MIAM && m >= MINUTE_DEBUT_MIAM)) &&
      (h < HEURE_FIN_MIAM || (h == HEURE_FIN_MIAM && m <= MINUTE_FIN_MIAM)))
  {
    // Serial.println("Dans la plage horaire de nourrissage.");
    maintenantSec = getRtcSecondsFromMidnight(); // Vérifier l'heure
    // Serial.println(maintenantSec);
    //  Si le chat est affamé : Verifier le délai depuis le dernier
    long deltaSecondes = maintenantSec - lastFeedtimeCroquettes; // interval de temps
    const unsigned long delay = FEED_DELAY_CROQUETTES_SEC + compteurAbsenceChat * SNOOZE_DELAY;
    // char message[56];
    // sprintf(message, "Maintenant: %d s - lastFeed: %d s - delta: %d s - delay: %d s", maintenantSec, lastFeedtimeCroquettes, deltaSecondes, delay);
    // Serial.println(message);
    if (deltaSecondes > 0 && (unsigned long)deltaSecondes >= delay) // Si le délai de 2H est écoulé
    {
      feedCat(1); // Donner des croquettes
    }
  }
  // --------- PLAGE HORAIRE (fin) --------- //

  // --------- Début des inputs bouton --------- //
  // 1. Lecture de l'état actuel du bouton
  int reading = digitalRead(BOUTON_PIN);
  // 2. Anti-rebond (Debounce) : On ne met à jour 'buttonState' que si l'état est stable
  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS)
  { // Si l'état a été stable pendant plus de DEBOUNCE_DELAY_MS
    if (reading != buttonState)
    {                        // on vient d'appuyer sur le bouton
      buttonState = reading; // mettre à jour 'buttonState'

      // 3. Gestion de l'événement PUSH (Appui)
      if (buttonState == LOW)
      { // LOW car on utilise INPUT_PULLUP (le bouton tire la broche à la masse)
        pressStartTime = millis();
        Serial.println("Le bouton est appuyé");
        // Serial.println("Allumage de la led");
        // digitalWrite(LED_PIN, HIGH); // Allume la LED
        //  Réinitialiser le compte de clic si le délai est dépassé
        if (millis() - lastClickTime > DOUBLE_CLICK_TIMEOUT_MS)
        {
          clickCount = 0;
        }
      }
      // 4. Gestion de l'événement RELEASE (Relâchement)
      else
      { // on vient de relacher le bouton
        releaseTime = millis();
        unsigned long pressDuration = releaseTime - pressStartTime;
        Serial.print("-> BOUTON RELACHE (Duree: ");
        Serial.print(pressDuration);
        Serial.println("ms)");
        // digitalWrite(LED_PIN, LOW); // Éteint la LED
        // Serial.println("Extinction de la led");

        // Détection de l'appui LONG
        if (pressDuration >= LONG_PRESS_MIN_MS)
        {
          // CAS n°0 : Appui TRES LONG
          Serial.println("--- APPUIS TRES LONG DETECTE ---");
          Serial.println("Synchronisation de l'horloge RTC avec le WiFi..");
          printMessage("Horloge", "Synchronisation de l'horloge RTC avec le WiFi..", 1);
          syncRTCFromWiFi();
        }
        else if (pressDuration >= SHORT_PRESS_MAX_MS)
        {
          // CAS n°1 : Appui LONG
          Serial.println("--- APPUIS LONG DETECTE ---");
          clickCount = 0; // Annule tout double-clic potentiel
          feedCat(0);     // Donner des croquinettes
        }
        // Détection de l'appui COURT (potentiel simple ou double clic)
        else if (pressDuration > DEBOUNCE_DELAY_MS)
        {
          clickCount++;
          lastClickTime = millis(); // Enregistre le moment du clic pour le timeout
        }
      }
    }
  }
  // 5. Détection du DOUBLE-CLIC ou du SIMPLE-CLIC
  // On vérifie s'il y a eu un ou plusieurs clics, et si le délai de double-clic est écoulé
  if (clickCount > 0 && (millis() - lastClickTime > DOUBLE_CLICK_TIMEOUT_MS))
  {
    if (clickCount == 2)
    {
      // CAS n°2 : Double clic
      Serial.println("--- DOUBLE APPUIS DETECTE ---");
      // Affiche le temps restant avant les prochaines croquettes
      char message[56];                                                                                            // Nombre de caractères max pour le message
      const unsigned int deltaMinutes = (lastFeedtimeCroquettes + FEED_DELAY_CROQUETTES_SEC - maintenantSec) / 60; // conversion en minutes
      sprintf(message, "Prochaines croquettes dans %d min", deltaMinutes);                                         // Prépare le message à afficher
      printMessage("Autocroq", message, 4);
    }
    else if (clickCount == 1)
    {
      // CAS n°3 : Simple clic
      Serial.println("--- APPUIS COURT (SIMPLE CLIC) DETECTE ---");
      displayScreen(5); // Afficher les dernières croquettes et croquinettes servies
    }
    clickCount = 0; // Réinitialisation
  }
  // Stocke l'état pour la prochaine itération
  lastButtonState = reading;
  // --------- Fin des inputs bouton --------- //

  // displayScreen(5); // Afficher les dernières croquettes et croquinettes servies
  oled.clearDisplay(); // Vidange du buffer de l'écran OLED
  printMessage("", "", 0);
  delay(1); // Délais de fin de boucle
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
      compteurAbsenceChat++;
      printMessage("No gazou", "Gazou est absent, distribution des croquettes reportée de 30min..", 2);
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
      compteurDeCroquettes++;                 // Mise à jour du compteur de croquettes
      Serial.println("Réinitialisation du compteur d'absence.");
      compteurAbsenceChat = 0;

      // Sauvegarder dans la mémoire persistante
      preferences.begin("croquinator", false);
      preferences.putULong("croquetteTime", lastFeedtimeCroquettes);
      preferences.putUInt("compteurCroquette", compteurDeCroquettes);
      preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.

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
        compteurDeCroquinettes++;                 // Mise à jour du compteur de croquinettes

        // Sauvegarder dans la mémoire persistante
        preferences.begin("croquinator", false);
        preferences.putULong("croquinetteTime", lastFeedtimecroquinettes);
        preferences.putUInt("compteurCroquinette", compteurDeCroquinettes);
        preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.

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
void getSavedSettings()
{
  preferences.begin("croquinator", true);
  lastFeedtimeCroquettes = preferences.getULong("croquetteTime", 0);     // 0 par défaut
  lastFeedtimecroquinettes = preferences.getULong("croquinetteTime", 0); // 0 par défaut
  compteurDeCroquettes = preferences.getUInt("compteurCroquette", 0);
  compteurDeCroquinettes = preferences.getUInt("compteurCroquinette", 0);
  preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.

  // Serial.println("Données récupérées depuis la mémoire :");
  // char message[50];
  // sprintf(message, "Croquettes   : %02d - lastTime: %lu", compteurDeCroquettes, lastFeedtimeCroquettes);
  // Serial.println(message);
  // sprintf(message, "Croquinettes : %02d - lastTime: %lu", compteurDeCroquinettes, lastFeedtimecroquinettes);
  // Serial.println(message);
}
// Initialize WiFi
void setupWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    printMessage("WiFi", "Connexion au WiFi...", 1);
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print('.');
      delay(1000);
    }
  }
  else
  {
    Serial.println("WiFi connected.");
    printMessage("WiFi", "WiFi connected.", 1);
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
void printMessage(const char *title, const char *message, unsigned int displayTimeSec)
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

  sprintf(outBuffer, "%02dh%02d", h, m);
}

void displayScreen(unsigned int displayTimeSec)
{
  const byte GRID_LINE_0 = 1;
  const byte GRID_LINE_1 = 20;
  const byte GRID_LINE_2 = 35;
  const byte GRID_LINE_3 = 50;

  const byte GRID_COL_0 = 0;
  const byte GRID_COL_1 = 75;
  const byte GRID_COL_2 = 95;

  char timeBuffer[9];  // Espace pour "HH:MM:SS\0"
  char dateBuffer[11]; // Espace pour "DD/MM/YYYY\0"
  // Formatage de l'heure et de la date dans les buffers de char
  sprintf(timeBuffer, "%02dh%02d", myRTC.hours, myRTC.minutes);
  sprintf(dateBuffer, "%02d/%02d/%04d", myRTC.dayofmonth, myRTC.month, myRTC.year);

  // Convertir en heure
  char heureCroquettes[6];
  char heureCroquinettes[6];
  char heureProchaineCroquettes[6];
  const int prochainCroqSec = lastFeedtimeCroquettes + FEED_DELAY_CROQUETTES_SEC;
  convertSecondsFromMidnightToTime(lastFeedtimeCroquettes, heureCroquettes);
  convertSecondsFromMidnightToTime(lastFeedtimecroquinettes, heureCroquinettes);
  convertSecondsFromMidnightToTime(prochainCroqSec, heureProchaineCroquettes);

  // Convertir en string
  char nombreDeCroquettes[3];   // Espace pour "00\0"
  char nombreDeCroquinettes[3]; // Espace pour "00\0"
  sprintf(nombreDeCroquettes, "%02d", compteurDeCroquettes);
  sprintf(nombreDeCroquinettes, "%02d", compteurDeCroquinettes);

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
  oled.print(nombreDeCroquettes);
  oled.setCursor(GRID_COL_2, GRID_LINE_1);
  oled.print(heureCroquettes);

  oled.setCursor(GRID_COL_0, GRID_LINE_2);
  oled.print("Croquinettes");
  oled.setCursor(GRID_COL_1, GRID_LINE_2);
  oled.print(nombreDeCroquinettes);
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

// Get time stamp
void printWiFiTime()
{
  Serial.println("Récupération de la date depuis le wifi...");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  // Convert to string
  char timeStr[19];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  // strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  Serial.print("Date actuelle: ");
  Serial.println(timeStr);
}

// Fonction de synchronisation principale
void syncRTCFromWiFi()
{
  // 0. Vérifier que le WiFi est connecté
  setupWiFi();
  // 1. Récupérer l'heure du WiFi
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    Serial.println("Synchronisation du RTC avec l'heure WiFi...");

    // 2. Transférer les données de la structure tm au RTC
    // La structure tm utilise 1900 comme année de base (tm_year est l'année - 1900).
    // Elle utilise 0-11 pour les mois (tm_mon) et 0-6 pour les jours de la semaine (tm_wday).

    // Détail des arguments de setDS1302Time (basé sur votre exemple) :
    // myRTC.setDS1302Time(sec, min, h, jour_semaine, jour_mois, mois, annee);

    // NOTE : L'année doit être complète (ex: 2024), selon certaines libs.
    // Si votre lib DS1302 n'attend que les deux derniers chiffres de l'année (ex: 24),
    // utilisez (timeinfo.tm_year % 100)

    myRTC.setDS1302Time(
        timeinfo.tm_sec,        // secondes (0-59)
        timeinfo.tm_min,        // minutes (0-59)
        timeinfo.tm_hour,       // heures (0-23)
        timeinfo.tm_wday,       // jour de la semaine (0=dimanche, 6=samedi)
        timeinfo.tm_mday,       // jour du mois (1-31)
        timeinfo.tm_mon + 1,    // mois (0-11) -> on ajoute 1 pour obtenir 1-12
        timeinfo.tm_year + 1900 // année (depuis 1900)
    );
    myRTC.updateTime();
    Serial.println("Synchronisation RTC réussie.");
    printMessage("Horloge", "Synchronisation de l'heure reussie", 1);
  }
  else
  {
    Serial.println("Synchronisation RTC échouée car l'heure WiFi n'a pas pu être récupérée.");
    printMessage("Horloge", "Synchronisation RTC echouee car l'heure WiFi n'a pas pu etre recuperee.", 5);
  }
}