// src/main.cpp

#include "config.h" //  includes, pins, constantes et variables globales

// --- OBJETS GLOBAUX ---
WiFiManager wifi(WIFI_SSID, WIFI_PASSWORD, AP_HOSTNAME);
OTAManager ota(OTA_HOSTNAME, OTA_PASSWORD, OTA_PORT);
RTCManager myRTC(DS1302_CLK_PIN, DS1302_DAT_PIN, DS1302_RST_PIN); // RTC module 2
Servo monServomoteur;                                             // Servomoteur
Preferences preferences;                                          // Persistent memory
OLEDDisplay oled(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_I2C_ADRESS);
InputBouton boutonTactile(BOUTON_PIN, LOW, INPUT);

// -------------------           DECLARATION DES FONCTIONS (début)           ------------------- /                                                           // (setup) Connecte la mémoire persistante
void setupWiFi();                                    // (setup) Connecte le wifi
void setupWebRoutes();                               // (setup) Initialise les pages web
void getSavedSettings();                             // (setup) Récupère la data de la mémoire persistante
void setupRtc();                                     // (setup) Initialise le module d'horloge
boolean syncRTCFromWiFi();                           // Synchronise la date RTC avec la date WiFi
void setupBoutons();                                 // (setup) Initialise les paramètres boutons
void setupScreen();                                  // (setup) Connecte l'écran OLED
void displayHomeScreen(unsigned int displayTimeSec); // Affiche l'écran de bord
void displayInfoScreen(unsigned int displayTimeSec); // Affiche les compteurs

// Fonctions Pour nourrir le chat
void setAutoMiam(bool isActivated);
void setMiamTime(unsigned int h, unsigned int m, String type);
boolean verifierRegime();
boolean detecterCroquettes();          // Détecte la présence de croquette, retourne (0) abscence || (1) présence
void openValve(unsigned int timeOpen); // Contrôle le servomoteur
int calculerMasseEngloutie();
void addHistoryPoint(unsigned long t, int m); // historique des distributions
void reinitialiserCompteurs();                // Réinitialise les compteurs
void feedCat(boolean grossePortion);          // Distribue les (0) Croquinettes || (1) Croquettes
void calibrerDistributeur(int repetitions, int startTimeOpen, int endTimeOpen, int step);
// -------------------           DECLARATION DES FONCTIONS (fin)           ------------------- /

// -------------------                INITIALISATION (début)                ------------------- /
void setup()
{
  DEBUG_INIT(SERIAL_BAUD_RATE);                                // Initialisation de la communication filaire                                              // wait until Arduino Serial Monitor opens
  DEBUG_PRINTLN(F("START Croquinator from " __DATE__ "\r\n")); //  Just to know which program is running

  setupScreen();                         // Initialisation de l'écran OLED
  getSavedSettings();                    // Récupération de la mémoire persistante
  setupWiFi();                           // Configuration du WiFi
  setupRtc();                            // Syncrhonisation de l'horloge interne
  monServomoteur.attach(SERVO_PIN);      // Configuration du Servomoteur
  monServomoteur.write(ANGLE_FERMETURE); // S'assure que la valve est fermée au démarrage
  setupBoutons();                        // Configuration des boutons

  calibrerDistributeur(1, 100, 1000, 100);
}
// -------------------                INITIALISATION (fin)                ------------------- /

// -------------------                BOUCLE LOOP (début)                ------------------- /
void loop()
{
  // DEBUG_PRINTLN("Début de la Boucle principale");
  // Appeler à chaque début de boucle
  wifi.checkConnection();
  wifi.handleClient();
  ota.handle();
  myRTC.update(); // Always update time
  oled.update();  // Loop Ecran OLED

  // --------- AutoCatFeed (début) --------- //
  // Vérifier la plage horaire toutes les secondes
  static unsigned long lastCheck = 0;
  if (autoMiamActivated && millis() - lastCheck > 1000)
  {
    lastCheck = millis();
    // Vérifier plage horaire
    const bool dansLaPlageHoraire = myRTC.isInTimeRange(heureDebutMiam, minuteDebutMiam,
                                                        heureFinMiam, minuteFinMiam);
    if (dansLaPlageHoraire)
    {
      // Serial.print("✓ Dans plage nourrissage");
      // Verifier le délai depuis le dernier croq
      long deltaSecondes = myRTC.getSecondsFromMidnight() - lastFeedTimeCroquettes; // interval de temps
      const unsigned long delay = delayDistributionCroquettesSec + compteurAbsenceChat * SNOOZE_DELAY_SEC;
      // char message[56];
      // sprintf(message, "Maintenant: %d s - lastFeed: %d s - delta: %d s - delay: %d s", maintenantSec, lastFeedTimeCroquettes, deltaSecondes, delay);
      // DEBUG_PRINTLN(message);
      if (deltaSecondes > 0 && (unsigned int)deltaSecondes >= delay) // Si le délai de 2H est écoulé
      {
        feedCat(1); // Donner des croquettes
      }
    }
  }
  // --------- AutoCatFeed (fin) --------- //

  // --------- Inputs bouton (debut) --------- //
  ButtonEvent event = boutonTactile.update(); // Appeler update() à chaque itération
  switch (event)                              // Traiter les événements
  {
  case BUTTON_PRESSED:
    DEBUG_PRINTLN("-> BOUTON PRESSE");
    break;

  case BUTTON_RELEASED:
    DEBUG_PRINT("-> BOUTON RELACHE (Duree: ");
    DEBUG_PRINT(boutonTactile.getPressDuration());
    DEBUG_PRINTLN("ms)");
    break;

  case BUTTON_SHORT_CLICK:
    DEBUG_PRINTLN("--- APPUIS COURT (SIMPLE CLIC) DETECTE ---");
    displayHomeScreen(DISPLAY_TIME_SEC); // Afficher les dernières croquettes et croquinettes servies
    break;

  case BUTTON_LONG_PRESS:
  {
    DEBUG_PRINTLN("--- APPUI LONG DETECTE ---");
    displayInfoScreen(DISPLAY_TIME_SEC); // Affiche les compteurs
    break;
  }

  case BUTTON_VERY_LONG_PRESS:
    DEBUG_PRINTLN("--- APPUIS TRES LONG DETECTE ---");
    syncRTCFromWiFi();
    break;

  case BUTTON_VERY_VERY_LONG_PRESS:
    DEBUG_PRINTLN("--- APPUIS TRES TRES LONG DETECTE ---");
    reinitialiserCompteurs();
    break;

  case BUTTON_MULTI_CLICK:
  {
    DEBUG_PRINTLN("--- MULTI CLIC DETECTE ---");
    uint8_t clics = boutonTactile.getClickCount();
    DEBUG_PRINT(clics);
    DEBUG_PRINTLN(" clics");
    if (clics == 2)
    {
      feedCat(0); // Donner des croquinettes
    }
    else if (clics == 3)
    {
      feedCat(1); //  Distribution dose normal de croquettes
    }
    break;
  }

  case BUTTON_NO_EVENT:
    // Rien à faire
    break;
  }
  // --------- Inputs bouton (fin) --------- //

  delay(1); // Délais de fin de boucle
}
// -------------------                BOUCLE LOOP (fin)                ------------------- /

// -------------------       FONCTIONS: Nourir le chat (début)       ------------------- /
void setAutoMiam(bool isActivated)
{
  if (isActivated == false)
  {
    autoMiamActivated = false;
    DEBUG_PRINTLN("[FitCat] Auto-miam désactivé");
    oled.printMessage("FitCat", "Desactivation de l'Auto-miam.", DISPLAY_TIME_SEC);
  }
  else
  {
    autoMiamActivated = true;
    DEBUG_PRINTLN("[FitCat] Auto-miam activé");
    oled.printMessage("FitCat", "Activation de l'Auto-miam.", DISPLAY_TIME_SEC);
  }
  // Sauvegarder dans la mémoire persistante
  preferences.begin("croquinator", false);
  preferences.putBool("autoMiam", autoMiamActivated);
  preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.
}
void setMiamTime(unsigned int h, unsigned int m, String type)
{
  DEBUG_PRINTLN("[FitCat] Paramétrage de la plage horaire..");
  preferences.begin("croquinator", false);
  if (type == "start")
  {
    heureDebutMiam = h;
    minuteDebutMiam = m;
    preferences.putUInt("heureDebutMiam", heureDebutMiam);
    preferences.putUInt("minuteDebutMiam", minuteDebutMiam);
    DEBUG_PRINTF("[FitCat] Nouveau début : %02dh%02d\n", h, m);
  }
  else if (type == "end")
  {
    heureFinMiam = h;
    minuteFinMiam = m;
    preferences.putUInt("heureFinMiam", heureFinMiam);
    preferences.putUInt("minuteFinMiam", minuteFinMiam);
    DEBUG_PRINTF("[FitCat] Nouvelle fin : %02dh%02d\n", h, m);
  }
  preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.
  oled.printMessage("FitCat", "Plage horaire mise à jour.", DISPLAY_TIME_SEC);
}
boolean verifierRegime()
{
  DEBUG_PRINT("Verification du régime du chat... ");
  masseEngloutieParLeChatEnG = calculerMasseEngloutie();
  DEBUG_PRINT(masseEngloutieParLeChatEnG);
  if (masseEngloutieParLeChatEnG < RATION_QUOTIDIENNE_G)
  {
    DEBUG_PRINTLN("g engloutis. El Gazou respecte son régime.");
    return true;
  }
  else
  {
    DEBUG_PRINTLN("g engloutis. El Gazou a suffisamment mangé !");
    return false;
  }
}
boolean detecterCroquettes()
{
  const boolean presence = digitalRead(IR_PIN);
  DEBUG_PRINT("Detection des croquettes... ");
  if (presence == LOW)
  {
    DEBUG_PRINTLN("Des croquetttes sont dans la gamelle");
    return true;
  }
  else
  {
    DEBUG_PRINTLN("Pas de croquetttes dans la gamelle");
    return false;
  }
}
void calibrerDistributeur(int repetitions, int startTimeOpen, int endTimeOpen, int step)
{
  DEBUG_PRINTLN("[FitCat] Calibration du distributeur");
  autoMiamActivated = false;

  for (int t = startTimeOpen; t <= endTimeOpen; t += step)
  {
    DEBUG_PRINTF("[Calibration] Temps d'ouverture %d ms %d répétitions - début dans 10sec..", t, repetitions);
    char message[60];
    sprintf(message, "Temps d'ouverture %d ms %d repetitions - debut dans 10sec..", t, repetitions);
    oled.printMessage("Calibrer", message, 10);
    delay(10 * 1000); // délais entre chaque temps d'ouverture
    for (int i = 1; i <= repetitions; i++)
    {
      DEBUG_PRINTF(" %d..", i);
      oled.printMessage("Calibrer", String(i), 1);
      openValve(t);
      delay(500); // délais entre chaque repetition
    }
    DEBUG_PRINTLN(" OK, mesurer masse totale !");
    oled.printMessage("Calibrer", "OK, mesurer masse totale !", 10);
    delay(10 * 1000);
  }

  DEBUG_PRINTLN("[FitCat] Calibration terminée");
  oled.printMessage("Calibrer", "Calibration terminee", 10);
}
void openValve(unsigned int timeOpen)
{
  DEBUG_PRINT("Ouverture de la valve (");
  DEBUG_PRINT(timeOpen);
  DEBUG_PRINTLN(" ms).");

  monServomoteur.write(ANGLE_OUVERTURE); // Ouvre
  delay(timeOpen);                       // Attend 0,5 seconde pour laisser tomber la portion
  monServomoteur.write(ANGLE_FERMETURE); // Ferme
}
int calculerMasseEngloutie()
{
  return compteurDeCroquettes * RATION_CROQUETTES_G + compteurDeCroquinettes * RATION_CROQUINETTES_G;
};
void addHistoryPoint(unsigned long time, int mass)
{
  if (historySize < MAX_HISTORY_POINTS)
  {
    feedingHistory[historySize] = {time, mass};
    historySize++;
  }
}
void optimiserDelayDistributionCroquettes()
{
  DEBUG_PRINT("Optimisation de la prochaine distribution.. ");
  masseEngloutieParLeChatEnG = calculerMasseEngloutie();
  const long finDeLaPlageDansSec = ((heureFinMiam * 60 + minuteFinMiam) * 60) - myRTC.getSecondsFromMidnight();
  const int nombreDistributionCroquettesRestant = (RATION_QUOTIDIENNE_G - masseEngloutieParLeChatEnG) / RATION_CROQUETTES_G;

  DEBUG_PRINT("Nombre de distributions restantes: ");
  DEBUG_PRINTLN(nombreDistributionCroquettesRestant);

  if (finDeLaPlageDansSec <= 0)
  {
    DEBUG_PRINTLN("Inutile, nous sommes en dehors de la plage horaire.");
  }
  else if (nombreDistributionCroquettesRestant <= 0)
  {
    DEBUG_PRINTLN("Inutile, toutes les croquettes ont été distribuées.");
  }
  else
  {
    delayDistributionCroquettesSec = finDeLaPlageDansSec / nombreDistributionCroquettesRestant;
    DEBUG_PRINTLN("Délai de distribution des croquettes ajusté");
    DEBUG_PRINT("Nouveau délai (min): ");
    DEBUG_PRINTLN(delayDistributionCroquettesSec / 60);
  }
}

void reinitialiserCompteurs()
{
  DEBUG_PRINTLN("Reinitialisation des compteurs.");
  compteurDeCroquettes = 0;
  compteurDeCroquinettes = 0;
  compteurAbsenceChat = 0;
  lastFeedTimeCroquettes = ((heureDebutMiam * 60 + minuteDebutMiam) * 60) - FEED_DELAY_CROQUETTES_SEC;
  lastFeedTimeCroquinettes = 0;
  delayDistributionCroquettesSec = FEED_DELAY_CROQUETTES_SEC;
  historySize = 0;                                    // Réinitialisation de l'historique
  addHistoryPoint(myRTC.getSecondsFromMidnight(), 0); // Point de départ à 0g

  preferences.begin("croquinator", true);
  lastFeedTimeCroquettes = preferences.putULong("croquetteTime", lastFeedTimeCroquettes);
  lastFeedTimeCroquinettes = preferences.putULong("croquinetteTime", lastFeedTimeCroquinettes);
  compteurDeCroquettes = preferences.putUInt("compteurCroquette", compteurDeCroquettes);
  compteurDeCroquinettes = preferences.putUInt("compteurCroquinette", compteurDeCroquinettes);
  preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.

  DEBUG_PRINTLN("Compteurs reinitialises.");
  oled.printMessage("Compteurs", "Reinitialisation des compteurs.", DISPLAY_TIME_SEC);
}
/* Fonction pour nourrir le chat
Vérifie la présence de croquettes et les distribue
@args
grossePortion : true (croquettes) | false (croquinettes)
*/
void feedCat(boolean grossePortion)
{
  DEBUG_PRINTLN("Nourrir le chat !");
  const boolean presenceDeCroquettes = detecterCroquettes(); // Vérifier si il y a des croquettes
  const boolean leRegimeEstRespecte = verifierRegime();      // Vérifier la quantité engloutée

  // CAS n°1 - Il y a déja des croquettes
  if (presenceDeCroquettes == true)
  {
    if (grossePortion == true)
    { // Croquettes
      DEBUG_PRINTLN("Distribution des croquettes reportee");
      compteurAbsenceChat++;
      oled.printMessage("No gazou", "Gazou est absent, distribution des croquettes reportee de 30min..", DISPLAY_TIME_SEC);
    }
    else
    { // Croquinettes
      DEBUG_PRINTLN("Pas de croquinettes pour les chats qui ne mangent pas");
      oled.printMessage("No way", "Il y a deja des croquettes dans la gamelles !", DISPLAY_TIME_SEC);
    }
  }
  // Fin du CAS n°1 - Il y a déja des croquettes

  // CAS n°2 - Le régime n'est pas respecté
  else if (leRegimeEstRespecte == false)
  {
    oled.printMessage("No Grazou", "Distribution annulee. Gazou a suffisamment mange aujourd'hui !", DISPLAY_TIME_SEC);
  }
  // Fin du CAS n°2 - Le régime n'est pas respecté

  // CAS n°3 - Il n'y a pas de croquettes et le régime est respecté
  else
  {
    DEBUG_PRINT("Distribution ");
    // Afficher l'image du chat
    oled.printImageCentered(IMAGE_CHAT, IMAGE_WIDTH, IMAGE_HEIGHT);
    //  Bonus: Prévenir le chat avec un son ou une led

    // CAS n°3a - Croquettes
    if (grossePortion == true)
    {
      DEBUG_PRINTLN(" des croquettes.");
      openValve(CROQUETTES);                                   // Nourrir le chat avec une portion complète
      lastFeedTimeCroquettes = myRTC.getSecondsFromMidnight(); // Met à jour le dernier temps de nourrissage
      compteurDeCroquettes++;                                  // Mise à jour du compteur de croquettes
      DEBUG_PRINTLN("Reinitialisation du compteur d'absence.");
      compteurAbsenceChat = 0;
      optimiserDelayDistributionCroquettes();
      addHistoryPoint(myRTC.getSecondsFromMidnight(), calculerMasseEngloutie()); // historique

      // Sauvegarder dans la mémoire persistante
      preferences.begin("croquinator", false);
      preferences.putULong("croquetteTime", lastFeedTimeCroquettes);
      preferences.putUInt("compteurCroquette", compteurDeCroquettes);
      preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.

      oled.printMessage("Miam", "El Gazou a eu sa dose", DISPLAY_TIME_SEC);
      DEBUG_PRINTLN("El Gazou a eu sa dose");
    }

    // CAS n°3b - Croquinettes
    else
    {
      DEBUG_PRINTLN(" des croquinettes.");
      // Vérifier que le delay des croquinettes est dépassé
      unsigned long deltaSecondes = myRTC.getSecondsFromMidnight() - lastFeedTimeCroquinettes; // interval de temps
      if (deltaSecondes >= FEED_DELAY_CROQUINETTES_SEC)                                        // Si le délai de 30 min est écoulé
      {
        DEBUG_PRINTLN("Délai écoulé, on peut donner une gourmandise/croquinette");
        openValve(CROQUINETTES);                                   // Nourrir le chat avec quelques croquettes
        lastFeedTimeCroquinettes = myRTC.getSecondsFromMidnight(); // Met à jour le dernier temps de gourmandise
        compteurDeCroquinettes++;                                  // Mise à jour du compteur de croquinettes
        optimiserDelayDistributionCroquettes();
        addHistoryPoint(myRTC.getSecondsFromMidnight(), calculerMasseEngloutie()); // historique

        // Sauvegarder dans la mémoire persistante
        preferences.begin("croquinator", false);
        preferences.putULong("croquinetteTime", lastFeedTimeCroquinettes);
        preferences.putUInt("compteurCroquinette", compteurDeCroquinettes);
        preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.

        DEBUG_PRINTLN("El gazou est servi !");
        oled.printMessage("Miaou", "El Gazou est servi !", DISPLAY_TIME_SEC);
      }
      else
      {
        DEBUG_PRINTLN("El gazou a deja eu sa gourmandise.");
        char message[56];                                                       // Nombre de caractères max pour le message
        const unsigned int deltaMinutes = deltaSecondes / 60;                   // conversion en minutes
        sprintf(message, "Dernieres Croquinettes il y a %d min", deltaMinutes); // Prépare le message à afficher
        oled.printMessage("No way", message, DISPLAY_TIME_SEC);
      }
    }
  }
  // Fin du CAS n°3 - Il n'y a pas de croquettes et le régime est respecté
};
// -------------------       FONCTIONS: Nourir le chat (fin)       ------------------- /

// -------------------       FONCTIONS: Setup boutons, mémoire et WiFi (début)       ------------------- /
void setupBoutons()
{
  // Initialiser le bouton
  boutonTactile.begin();
  // Configuration optionnelle des seuils (sinon valeurs par défaut)
  boutonTactile.setDebounce(30);           // Anti rebond
  boutonTactile.setShortPressMax(500);     // Appui court max
  boutonTactile.setLongPressMin(2000);     // Appui long min
  boutonTactile.setMultiClickTimeout(400); // Délai entre clics
  boutonTactile.setMaxClickCount(3);       // Détection jusqu'au triple-clic
}
void getSavedSettings()
{
  preferences.begin("croquinator", true);
  autoMiamActivated = preferences.getBool("autoMiam", autoMiamActivated);
  heureDebutMiam = preferences.getUInt("heureDebutMiam", heureDebutMiam);
  minuteDebutMiam = preferences.getUInt("minuteDebutMiam", minuteDebutMiam);
  heureFinMiam = preferences.getUInt("heureFinMiam", heureFinMiam);
  minuteFinMiam = preferences.getUInt("minuteFinMiam", minuteFinMiam);
  lastFeedTimeCroquettes = preferences.getULong("croquetteTime", 0);     // 0 par défaut
  lastFeedTimeCroquinettes = preferences.getULong("croquinetteTime", 0); // 0 par défaut
  compteurDeCroquettes = preferences.getUInt("compteurCroquette", 0);
  compteurDeCroquinettes = preferences.getUInt("compteurCroquinette", 0);
  preferences.end(); // Ferme l'accès à la mémoire. C'est CRUCIAL.
  oled.printMessage("Memory", "Donnees recuperees depuis la memoire", DISPLAY_TIME_SEC);

  // DEBUG_PRINTLN("Données récupérées depuis la mémoire :");
  // char message[50];
  // sprintf(message, "Croquettes   : %02d - lastTime: %lu", compteurDeCroquettes, lastFeedTimeCroquettes);
  // DEBUG_PRINTLN(message);
  // sprintf(message, "Croquinettes : %02d - lastTime: %lu", compteurDeCroquinettes, lastFeedTimeCroquinettes);
  // DEBUG_PRINTLN(message);
}
void setupWiFi()
{
  // Initialiser WiFi
  wifi.begin();
  // wifi.setStateChangeCallback(onWiFiStateChange);
  wifi.setConnectionTimeout(30000);
  wifi.setMaxReconnectAttempts(5);

  // Tenter la connexion
  DEBUG_PRINT("Tentative de connexion WiFi...");
  oled.printMessage("WiFi", "Connexion au WiFi...", DISPLAY_TIME_SEC);
  if (!wifi.connect())
  {
    DEBUG_PRINTLN("✗ Échec de connexion au WiFi principal");
    DEBUG_PRINTLN("→ Démarrage du mode Access Point de secours");
    oled.printMessage("WiFi", "Echec de connexion au WiFi. Demarrage du mode Access Point...", DISPLAY_TIME_SEC);

    // Démarrer en mode AP si échec de connexion
    if (wifi.startAP(AP_SSID, AP_PASSWORD))
    {
      DEBUG_PRINTLN("✓ Mode AP activé");
      DEBUG_PRINT("SSID: ");
      DEBUG_PRINTLN(AP_SSID);
      DEBUG_PRINT("IP: ");
      DEBUG_PRINTLN(WiFi.softAPIP());
      oled.printMessage("WiFi", "Mode AP active.", DISPLAY_TIME_SEC);
    }
  }
  else
  {
    DEBUG_PRINTLN("WiFi connected.");
    oled.printMessage("WiFi", "WiFi connected.", DISPLAY_TIME_SEC);

    // Activer NTP
    wifi.enableNTP(NTP_SERVER, GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC);
  }

  // Démarrer le serveur web
  if (wifi.startWebServer(80))
  {
    setupWebRoutes();
    DEBUG_PRINTLN("\n✓ Serveur web démarré");
    oled.printMessage("WiFi", "Serveur web online.", DISPLAY_TIME_SEC);

    if (wifi.isConnected())
    {
      DEBUG_PRINT("URL: http://");
      DEBUG_PRINTLN(wifi.getIP());
    }
    else
    {
      DEBUG_PRINT("URL AP: http://");
      DEBUG_PRINTLN(WiFi.softAPIP());
    }
  }

  // Initialisation du Service OTA
  if (!ota.begin())
  {
    DEBUG_PRINTLN(F("[Erreur] OTA non initialisé"));
  }
}
int getWiFiSignalLevel()
{
  int rssi = wifi.getRSSI();
  DEBUG_PRINT("Niveau du signal WiFi (dBm): ");
  DEBUG_PRINTLN(rssi);
  if (rssi > -55)
  {
    return 4; // Excellent
  }
  else if (rssi > -70)
  {
    return 3; // Bon
  }
  else if (rssi > -80)
  {
    return 2; // Moyen
  }
  else if (rssi > -90)
  {
    return 1; // Faible
  }
  else
  {
    return 0; // Très faible/Inutilisable
  }
}
// -------------------       FONCTIONS: Setup mémoire et WiFi (fin)       ------------------- /

// -------------------       FONCTIONS: Ecran OLED (début)       ------------------- /
// Initialisation de l'écran OLED
void setupScreen()
{
  if (!oled.begin())
  {
    DEBUG_PRINTLN("Erreur d'initialisation OLED !");
  }
  else
  {
    DEBUG_PRINTLN("Chargement du système...");
    for (int i = 0; i <= 100; i += 10)
    {
      oled.printImageWithProgress(IMAGE_CHAT, IMAGE_WIDTH, IMAGE_HEIGHT, i);
      delay(50);
    }
    delay(100);
  }
}

void displayHomeScreen(unsigned int displayTimeSec)
{
  oled.clear();

  // En-tête avec indicateur de WiFi
  oled.printDate(myRTC.getDayOfMonth(), myRTC.getMonth(), myRTC.getYear(), ALIGN_LEFT, 0);
  const int wifiSignal = getWiFiSignalLevel();
  oled.drawWifiSignal(115, 0, wifiSignal);

  // Heure actuelle
  oled.printTime(myRTC.getHour(), myRTC.getMinute(), ALIGN_CENTER, 17, 2);

  // Prochaine distribution
  int prochainCroqSec = (lastFeedTimeCroquettes + delayDistributionCroquettesSec + compteurAbsenceChat * SNOOZE_DELAY_SEC);
  oled.printTextAligned("Prochain croq:", ALIGN_LEFT, 40);
  oled.printTextAligned(myRTC.formatSecondsToTime(prochainCroqSec, false), ALIGN_RIGHT, 40);

  // Barre de progression
  const float progress = masseEngloutieParLeChatEnG * 100 / RATION_QUOTIDIENNE_G;
  DEBUG_PRINT("AutoFeed (%) : ");
  DEBUG_PRINTLN(progress);
  oled.drawProgressBarBottom(progress, true);

  oled.refresh();
  oled.startTimer(displayTimeSec);
}

void displayInfoScreen(unsigned int displayTimeSec)
{
  const int wifiSignal = getWiFiSignalLevel();

  oled.clear();

  // En-tête avec indicateur de WiFi
  oled.printText("Compteurs", 0, 0, 1);
  oled.drawWifiSignal(115, 0, wifiSignal);

  oled.printTextAligned("Croquettes", ALIGN_LEFT, 20);
  oled.printValue(" - ", compteurDeCroquettes, 0, "", 30);
  oled.printTextAligned(myRTC.formatSecondsToTime(lastFeedTimeCroquettes, false), ALIGN_RIGHT, 30);
  // oled.printTime(8, 21, ALIGN_RIGHT, 26, 1);

  oled.printTextAligned("Croquinettes", ALIGN_LEFT, 46);
  oled.printValue(" - ", compteurDeCroquinettes, 0, "", 56);
  oled.printTextAligned(myRTC.formatSecondsToTime(lastFeedTimeCroquinettes, false), ALIGN_RIGHT, 56);
  // oled.printTime(8, 17, ALIGN_RIGHT, 56, 1);

  oled.refresh();
  oled.startTimer(displayTimeSec);
}
// -------------------       FONCTIONS: Ecran OLED (fin)      ------------------- /

// -------------------       FONCTIONS: RTC (début)       ------------------- /
void setupRtc()
{
  // Initialiser le RTC
  myRTC.begin();
  myRTC.setDebugMode(DEBUG_MODE);

  //  Configurer la date
  // myRTC.setDateTime(0, 54, 22, 4, 11, 12, 2025);
  syncRTCFromWiFi();

  // Afficher l'heure actuelle
  DEBUG_PRINTLN(F("\n--- Heure actuelle ---"));
  myRTC.printDateTime();

  // ==================== ALARMES ====================
  DEBUG_PRINTLN(F("\n--- Configuration des alarmes ---"));
  // Alarme à 8h00
  // myRTC.addAlarm(8, 0, []()
  //              { DEBUG_PRINTLN(F("🔔 Alarme 8h00 : Début de journée !")); });

  // ==================== CALLBACK MINUIT ====================
  myRTC.setMidnightCallback([]()
                            {
                             DEBUG_PRINTLN(F("\n⏰ MINUIT - Nouveau jour !"));
                            // Resynchroniser avec NTP si WiFi disponible
                             syncRTCFromWiFi();
                                 reinitialiserCompteurs(); });

  DEBUG_PRINTLN(F("✓ Alarmes configurées\n"));
  DEBUG_PRINTLN(F("\n=== Système prêt ===\n"));
}
boolean syncRTCFromWiFi()
{
  oled.printMessage("Horloge", "Synchronisation de l'horloge RTC avec le WiFi..", DISPLAY_TIME_SEC);
  const bool isSync = myRTC.syncFromNTP(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC);
  if (myRTC.getYear() == 2000)
  {
    DEBUG_PRINTLN("Synchronisation RTC échouée, vérifier la batterie ou le branchement.");
    oled.printMessage("Horloge", "Echec de synchronisation de l'heure RTC, verifier la batterie ou le branchement.", 15 * 60);
    return false;
  }
  if (isSync)
  {
    oled.printMessage("Horloge", "Synchronisation de l'heure reussie", DISPLAY_TIME_SEC);
    return true;
  }
  else
  {
    oled.printMessage("Horloge", "Synchronisation RTC echouee car l'heure WiFi n'a pas pu etre recuperee.", DISPLAY_TIME_SEC);
    return false;
  }
}
// -------------------       FONCTIONS: RTC (fin)       ------------------- /

// -------------------       WEBROUTES (debut)       ------------------- /
void setupWebRoutes()
{
  // Pages par défaut (home /status et /info)
  wifi.enableDefaultPages(true);

  // Dashboard page
  wifi.on("/dashboard", [](WebServerType &server)
          { server.send(200, "text/html", DashboardPage::getHTML()); });

  // API de données (Output pour l'UI)
  wifi.on("/api/data", [](WebServerType &server)
          {
             //DEBUG_PRINTLN("[Web] Nouvelle requête : /api/data");

             const unsigned long maintenantSec = myRTC.getSecondsFromMidnight();
             int dernieresCroquettes = maintenantSec - lastFeedTimeCroquettes;
             int dernieresCroquinettes = (maintenantSec - lastFeedTimeCroquinettes);
             int prochainCroq = (lastFeedTimeCroquettes + delayDistributionCroquettesSec + compteurAbsenceChat * SNOOZE_DELAY_SEC - maintenantSec);

             JsonDocument doc;
             doc["nbCroquettes"] = compteurDeCroquettes;
             doc["nbCroquinettes"] = compteurDeCroquinettes;
             doc["hCroquettes"] = myRTC.formatDuration(dernieresCroquettes);
             doc["hCroquinettes"] = myRTC.formatDuration(dernieresCroquinettes);
             doc["hNextCroquettes"] = myRTC.formatDuration(prochainCroq);
             doc["delay"] = myRTC.formatDuration(delayDistributionCroquettesSec);
             doc["mass"] = masseEngloutieParLeChatEnG;
             doc["ration"] = RATION_QUOTIDIENNE_G;
             doc["autoMiam"] = autoMiamActivated;

             // Formatage de la plage horaire pour les inputs (ex: "07:30")
             char timeBuf[6];
             sprintf(timeBuf, "%02d:%02d", heureDebutMiam, minuteDebutMiam);
             doc["timeStart"] = String(timeBuf);
             sprintf(timeBuf, "%02d:%02d", heureFinMiam, minuteFinMiam);
             doc["timeEnd"] = String(timeBuf);

             // Ajout de l'historique au JSON
             JsonArray hist = doc["history"].to<JsonArray>();
             for (int i = 0; i < historySize; i++)
             {
               JsonObject point = hist.add<JsonObject>();
               point["t"] = feedingHistory[i].timestamp;
               point["m"] = feedingHistory[i].cumulativeMass;
             }

             String output;
             serializeJson(doc, output);
             // DEBUG_PRINTLN(output);
             server.send(200, "application/json", output); });

  //  API de commandes (Input depuis l'UI)
  wifi.on("/setAutomiam", [](WebServerType &server)
          {
            DEBUG_PRINTLN("[Web] Nouvelle requête : /setAutomiam");
            // Désactiver les distribution
            int val = server.arg("v").toInt();
            setAutoMiam(val);
            server.send(200, "text/plain", "OK"); });
  wifi.on("/feedCat", [](WebServerType &server)
          {
            DEBUG_PRINTLN("[Web] Nouvelle requête : /feedCat");
        int val = server.arg("v").toInt();
        feedCat(val);
        server.send(200, "text/plain", "OK"); });
  wifi.on("/reset", [](WebServerType &server)
          { 
            DEBUG_PRINTLN("[Web] Nouvelle requête : /reset");
          reinitialiserCompteurs();
          server.send(200, "text/plain", "OK"); });
  // Nouvelle route : Réglage des horaires
  wifi.on("/setMiamTime", [](WebServerType &server)
          {
            DEBUG_PRINTLN("[Web] Nouvelle requête : /setMiamTime");
    String type = server.arg("type"); // "start" ou "end"
    String val = server.arg("val");   // Format "HH:MM"
    
    if (val.length() == 5) {
        int h = val.substring(0, 2).toInt();
        int m = val.substring(3, 5).toInt();
        setMiamTime(h, m, type);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Format invalide");
    } });

  // Scan des réseaux
  wifi.on("/scan", [](WebServerType &server)
          {
            DEBUG_PRINTLN("[Web] Nouvelle requête : /scan");
            wifi.scanNetworks(); // 'wifi' doit être capturé ou global
    server.send(200, "application/json", wifi.getScannedNetworkJSON()); });

  // Redémarrer l'ESP
  wifi.on("/restart", [](WebServerType &server)
          {
            DEBUG_PRINTLN("[Web] Nouvelle requête : /restart");
  server.send(200, "text/plain", "Redémarrage...");
  delay(1000);
  ESP.restart(); });
}
// -------------------       WEBROUTES (fin)       ------------------- /