// inludes/config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Servo.h>       // Servomoteur
#include <Preferences.h> // Persistent memory
#include <ArduinoJson.h>

// --- INCLUSIONS PERSO ---
#include <WiFiManager.h>
#include <OTAManager.h>
#include <RTCManager.h>
#include <OLEDDisplay.h>
#include <InputBouton.h>
#include "DashboardPage.h"

#include "debug.h"
#include "images.h" //  image de chat
#include "secrets.h"

// --- WIFI CONFIG ---
const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

// --- SERVOMOTEUR ---
#define SERVO_PIN D3
const int ANGLE_OUVERTURE = 180;       // Angle pour ouvrir la valve
const int ANGLE_FERMETURE = 60;        // Angle pour fermer la valve
const unsigned int CROQUINETTES = 111; // temps  (ms) ouverture rapide
const unsigned int CROQUETTES = 500;   // temps (ms) ouverture longue

// --- RTC (DS1302) ---
#define DS1302_CLK_PIN D7
#define DS1302_DAT_PIN D6
#define DS1302_RST_PIN D5

// Capteur de présence de croquettes
#define IR_PIN D0 // Pin du capteur ir

// --- ECRAN OLED ---
#define SCREEN_WIDTH 128        // Taille de l'écran OLED, en pixel, au niveau de sa largeur
#define SCREEN_HEIGHT 64        // Taille de l'écran OLED, en pixel, au niveau de sa hauteur
#define OLED_RESET_PIN -1       // Reset de l'OLED partagé avec l'Arduino (d'où la valeur à -1, et non un numéro de pin)
#define OLED_I2C_ADRESS 0x3C    // Adresse de "mon" écran OLED sur le bus i2c (généralement égal à 0x3C ou 0x3D)
const int DISPLAY_TIME_SEC = 5; // Temps d'affichage en secondes

// Bouton Tactile TTP223
#define BOUTON_PIN D8 // Pin du bouton

// --- PARAMETRES TIMER ---
// Définition de la plage horaire
int heureDebutMiam = 7;
int minuteDebutMiam = 30;
int heureFinMiam = 23;
int minuteFinMiam = 15;
boolean autoMiamActivated = true;                            // Activer ou désactiver la distribution automatique
const unsigned long FEED_DELAY_CROQUETTES_SEC = 2 * 60 * 60; // Délai minimum entre deux distributions (2 heures)
const unsigned long FEED_DELAY_CROQUINETTES_SEC = 60;        // 30 * 60;   // Délai minimum entre deux distributions rapides (30 minutes)
const unsigned long SNOOZE_DELAY_SEC = 30;                   // 30 * 60;              // Délai en cas de présence de croquettes (30 minutes)
unsigned long lastFeedTimeCroquettes = 0;                    // Dernier temps (en secondes depuis minuit) où le chat a été nourri avec des croquettes
unsigned long lastFeedTimeCroquinettes = 0;                  // Dernier temps (en secondes depuis minuit) où le chat a été nourri avec quelques croquinettes
unsigned int compteurAbsenceChat = 0;                        // Nombre de reports de distributions de croquettes
unsigned int compteurDeCroquettes = 0;                       // Nombre de distributions de croquettes pour ce jour
unsigned int compteurDeCroquinettes = 0;                     // Nombre de distributions de croquinettes pour ce jour

// --- PARAMETRES FitCat ---
const int RATION_QUOTIDIENNE_G = 75; // Ration quotidienne (en g) idéale pour El Gazou
const int RATION_CROQUETTES_G = 5;   // Ration croquettes (en g) par distribution
const int RATION_CROQUINETTES_G = 1; // Ration croquinettes (en g) par distribution
int masseEngloutieParLeChatEnG = 0;
unsigned long delayDistributionCroquettesSec = FEED_DELAY_CROQUETTES_SEC;

// --- HISTORIQUE ---
struct FeedingTime
{
    unsigned long timestamp; // Secondes depuis minuit
    int cumulativeMass;      // Masse totale à cet instant
};
const int MAX_HISTORY_POINTS = 30; // Suffisant pour une journée
FeedingTime feedingHistory[MAX_HISTORY_POINTS];
int historySize = 0;

#endif