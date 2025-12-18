# RTCManager Library

Librairie complète pour gérer le module **RTC DS1302** avec de nombreuses fonctionnalités avancées. Basée sur virtuabotixRTC avec extensions.

## ✨ Caractéristiques

### Fonctionnalités de base

- ✅ Configuration date/heure simple
- ✅ Lecture et formatage
- ✅ Conversions temporelles
- ✅ Comparaisons et calculs

### Fonctionnalités avancées

- ✅ **Alarmes multiples** (jusqu'à 5)
- ✅ **Callback à minuit** automatique
- ✅ **Vérification de plages horaires**
- ✅ **Synchronisation NTP** (ESP8266)
- ✅ Noms de jours/mois en français
- ✅ Formatage personnalisable
- ✅ Unix timestamp
- ✅ Détection weekend/semaine

## 📦 Installation

```
lib/RTCManager/
├── RTCManager.h
├── RTCManager.cpp
└── examples/
    └── CompleteRTC/CompleteRTC.ino
```

**Dépendance requise :**

- virtuabotixRTC

```ini
lib_deps =
    virtuabotixRTC
```

## 🚀 Utilisation rapide

### Configuration initiale

```cpp
#include <RTCManager.h>

// Broches (ESP8266)
#define RTC_CLK_PIN D7
#define RTC_DAT_PIN D6
#define RTC_RST_PIN D5

RTCManager rtc(RTC_CLK_PIN, RTC_DAT_PIN, RTC_RST_PIN);

void setup() {
  Serial.begin(9600);

  rtc.begin();

  // ⚠️ CONFIGURER UNE SEULE FOIS puis commenter !
  // Format: seconde, minute, heure, jour_semaine, jour_mois, mois, année
  rtc.setDateTime(0, 54, 22, 4, 11, 12, 2025);
}

void loop() {
  rtc.update();

  Serial.println(rtc.getTimeString());
  delay(1000);
}
```

### Tes fonctions (version améliorée)

```cpp
// 1. Vérifier plage horaire (ta fonction)
boolean verifierPlageHoraire() {
  return rtc.isInTimeRange(8, 0, 20, 0); // 8h00 - 20h00
}

// 2. Récupérer les secondes depuis minuit (ta fonction)
unsigned long getSecondsFromMidnight() {
  return rtc.getSecondsFromMidnight();
}

// 3. Formater en HH:MM (ta fonction)
String formatTime(unsigned long seconds) {
  return rtc.formatSecondsToTime(seconds);
}

// 4. Callback à minuit (ta fonction)
void setup() {
  rtc.setMidnightCallback([]() {
    Serial.println("Minuit !");
    reinitialiserCompteurs();
    // Optionnel : resync NTP
    // rtc.syncFromNTP(3600, 3600);
  });
}
```

## 📚 API Complète

### Initialisation

```cpp
RTCManager(clk, dat, rst);
bool begin();
void setDebugMode(enable);
```

### Configuration de l'heure

```cpp
void setDateTime(s, m, h, dow, dom, month, year);
void setTime(hour, minute, second);
void setDate(day, month, year);
```

**Exemple :**

```cpp
// Configurer le 11 décembre 2025 à 22h54m00s (Jeudi)
rtc.setDateTime(0, 54, 22, 4, 11, 12, 2025);

// Modifier uniquement l'heure
rtc.setTime(14, 30, 0);

// Modifier uniquement la date
rtc.setDate(25, 12, 2025);
```

### Synchronisation NTP (ESP8266)

```cpp
bool syncFromNTP(gmtOffset = 0, dstOffset = 0);
```

**Exemple :**

```cpp
// Paris (UTC+1, heure d'été)
if (rtc.syncFromNTP(3600, 3600)) {
  Serial.println("✓ Synchronisé avec NTP");
}
```

### Lecture de l'heure

```cpp
void update();                    // Mettre à jour (appeler dans loop)
DateTime getDateTime();           // Structure complète
String getTimeString(format);     // "HH:MM:SS"
String getDateString(format);     // "DD/MM/YYYY"
String getDateTimeString();       // "DD/MM/YYYY HH:MM:SS"
```

**Exemple :**

```cpp
rtc.update();

Serial.println(rtc.getTimeString());           // "14:30:25"
Serial.println(rtc.getDateString());           // "11/12/2025"
Serial.println(rtc.getDateTimeString());       // "11/12/2025 14:30:25"
Serial.println(rtc.getTimeString("%02d:%02d")); // "14:30"
```

### Composants individuels

```cpp
uint8_t getSecond();
uint8_t getMinute();
uint8_t getHour();
uint8_t getDayOfWeek();    // 1=Dim, 2=Lun, ..., 7=Sam
uint8_t getDayOfMonth();
uint8_t getMonth();
uint16_t getYear();
```

**Exemple :**

```cpp
int h = rtc.getHour();
int m = rtc.getMinute();
Serial.printf("Il est %02d:%02d\n", h, m);
```

### Conversions temporelles

```cpp
unsigned long getSecondsFromMidnight();      // Secondes depuis 00:00
unsigned long getSecondsFromEpoch();         // Unix timestamp
void setFromSecondsFromEpoch(seconds);       // Depuis timestamp
```

**Exemple :**

```cpp
unsigned long sec = rtc.getSecondsFromMidnight();
Serial.print("Secondes depuis minuit: ");
Serial.println(sec);

// Unix timestamp
unsigned long epoch = rtc.getSecondsFromEpoch();
Serial.print("Timestamp: ");
Serial.println(epoch);
```

### Formatage

```cpp
String formatSecondsToTime(seconds, showSeconds = false);
String formatDuration(seconds);
```

**Exemple :**

```cpp
unsigned long sec = 7530; // 2h 5m 30s

Serial.println(rtc.formatSecondsToTime(sec));       // "02h05"
Serial.println(rtc.formatSecondsToTime(sec, true)); // "02h05m30s"
Serial.println(rtc.formatDuration(sec));            // "2h 5m 30s"
```

### Plages horaires

```cpp
bool isInTimeRange(startH, startM, endH, endM);
bool isInTimeRange(TimeRange& range);
```

**Exemple :**

```cpp
// Simple
if (rtc.isInTimeRange(8, 0, 20, 0)) {
  Serial.println("Dans la plage 8h-20h");
}

// Avec structure
TimeRange miam = {8, 0, 20, 0};
if (rtc.isInTimeRange(miam)) {
  Serial.println("C'est l'heure de manger !");
}

// Plage traversant minuit (22h-6h)
if (rtc.isInTimeRange(22, 0, 6, 0)) {
  Serial.println("C'est la nuit");
}
```

### Jours de la semaine

```cpp
String getDayName(shortName = false);
String getMonthName(shortName = false);
bool isWeekend();
bool isWeekday();
```

**Exemple :**

```cpp
Serial.println(rtc.getDayName());        // "Jeudi"
Serial.println(rtc.getDayName(true));    // "Jeu"
Serial.println(rtc.getMonthName());      // "Décembre"
Serial.println(rtc.getMonthName(true));  // "Déc"

if (rtc.isWeekend()) {
  Serial.println("C'est le weekend !");
}
```

### Alarmes (jusqu'à 5)

```cpp
int addAlarm(hour, minute, callback);
void removeAlarm(alarmId);
void enableAlarm(alarmId);
void disableAlarm(alarmId);
void clearAllAlarms();
```

**Exemple :**

```cpp
// Ajouter des alarmes
int alarm1 = rtc.addAlarm(8, 0, []() {
  Serial.println("🔔 8h00 - Réveil !");
});

int alarm2 = rtc.addAlarm(12, 0, []() {
  Serial.println("🔔 12h00 - Déjeuner !");
});

int alarm3 = rtc.addAlarm(20, 0, []() {
  Serial.println("🔔 20h00 - Dîner !");
});

// Désactiver temporairement une alarme
rtc.disableAlarm(alarm1);

// Réactiver
rtc.enableAlarm(alarm1);

// Supprimer une alarme
rtc.removeAlarm(alarm2);

// Tout effacer
rtc.clearAllAlarms();
```

### Callback à minuit

```cpp
void setMidnightCallback(callback);
```

**Exemple :**

```cpp
rtc.setMidnightCallback([]() {
  Serial.println("⏰ MINUIT - Nouveau jour !");
  compteurJournalier = 0;
  sauvegarderDonnees();
  // Resync NTP si WiFi
  // rtc.syncFromNTP(3600, 3600);
});
```

### Calculs de temps

```cpp
long getTimeDifferenceSeconds(h1, m1, h2, m2);
bool isBefore(h1, m1, h2, m2);
bool isAfter(h1, m1, h2, m2);
```

**Exemple :**

```cpp
// Différence entre 8h30 et 14h45
long diff = rtc.getTimeDifferenceSeconds(8, 30, 14, 45);
Serial.print("Différence: ");
Serial.println(rtc.formatDuration(diff));

// Comparaisons
if (rtc.isBefore(8, 0, 12, 0)) {
  Serial.println("8h est avant 12h");
}
```

### Comparaisons de dates

```cpp
bool isSameDay(dt1, dt2);
bool isSameMonth(dt1, dt2);
int daysBetween(dt1, dt2);
```

**Exemple :**

```cpp
DateTime now = rtc.getDateTime();
DateTime noel = {0, 0, 0, 4, 25, 12, 2025};

if (rtc.isSameDay(now, noel)) {
  Serial.println("C'est Noël !");
}

int days = rtc.daysBetween(now, noel);
Serial.print("Jours avant Noël: ");
Serial.println(days);
```

### Utilitaires

```cpp
void printInfo();
void printDateTime();
virtuabotixRTC* getRTC(); // Accès direct
```

## 💡 Exemples complets

### Dashboard horaire complet

```cpp
void afficherDashboard() {
  Serial.println("\n========== DASHBOARD ==========");

  rtc.printDateTime();

  Serial.print("Depuis minuit: ");
  Serial.println(rtc.formatSecondsToTime(rtc.getSecondsFromMidnight()));

  Serial.print("Weekend: ");
  Serial.println(rtc.isWeekend() ? "Oui" : "Non");

  Serial.print("Plage nourrissage: ");
  Serial.println(rtc.isInTimeRange(8, 0, 20, 0) ? "✓ Oui" : "✗ Non");

  Serial.println("===============================\n");
}
```

### Système de nourrissage automatique (ton cas)

```cpp
#define HEURE_DEBUT 8
#define MINUTE_DEBUT 0
#define HEURE_FIN 20
#define MINUTE_FIN 0

int compteurNourritures = 0;
bool dernierEtatPlage = false;

void setup() {
  rtc.begin();

  // Callback minuit
  rtc.setMidnightCallback([]() {
    Serial.println("Minuit - Reset compteurs");
    compteurNourritures = 0;
    // Resync avec NTP si dispo
    #ifdef ESP8266
    if (WiFi.status() == WL_CONNECTED) {
      rtc.syncFromNTP(3600, 3600);
    }
    #endif
  });

  // Alarmes de rappel
  rtc.addAlarm(HEURE_DEBUT, MINUTE_DEBUT, []() {
    Serial.println("🔔 Début plage nourrissage");
  });

  rtc.addAlarm(HEURE_FIN, MINUTE_FIN, []() {
    Serial.println("🔔 Fin plage nourrissage");
  });
}

void loop() {
  rtc.update();

  // Vérifier plage horaire
  bool dansPlage = rtc.isInTimeRange(HEURE_DEBUT, MINUTE_DEBUT,
                                      HEURE_FIN, MINUTE_FIN);

  // Détecter entrée/sortie de plage
  if (dansPlage && !dernierEtatPlage) {
    Serial.println("→ Entrée dans la plage");
  } else if (!dansPlage && dernierEtatPlage) {
    Serial.println("← Sortie de la plage");
  }

  dernierEtatPlage = dansPlage;

  // Nourrir si dans la plage et bouton pressé
  if (dansPlage && boutonPresse) {
    nourrir();
    compteurNourritures++;
  }
}
```

### Synchronisation NTP quotidienne

```cpp
void setup() {
  wifi.begin();
  wifi.connect();
  rtc.begin();

  // Sync NTP à chaque minuit
  rtc.setMidnightCallback([]() {
    Serial.println("Minuit - Sync NTP");

    if (WiFi.status() == WL_CONNECTED) {
      if (rtc.syncFromNTP(3600, 3600)) {
        Serial.println("✓ RTC synchronisé avec NTP");
      } else {
        Serial.println("✗ Échec sync NTP");
      }
    }
  });
}
```

## 🔧 Connexion matérielle

### DS1302 → ESP8266

| DS1302 | ESP8266     | Description  |
| ------ | ----------- | ------------ |
| VCC    | 3.3V        | Alimentation |
| GND    | GND         | Masse        |
| CLK    | D7 (GPIO13) | Horloge      |
| DAT    | D6 (GPIO12) | Données      |
| RST    | D5 (GPIO14) | Reset        |

### DS1302 → Arduino

| DS1302 | Arduino |
| ------ | ------- |
| VCC    | 5V      |
| GND    | GND     |
| CLK    | 6       |
| DAT    | 7       |
| RST    | 8       |

## ⚠️ Important

### 1. Configuration initiale

```cpp
// ⚠️ Exécuter UNE SEULE FOIS puis commenter !
rtc.setDateTime(0, 30, 14, 5, 12, 12, 2025);
```

### 2. Pile CR2032

Le DS1302 a une pile de secours. Quand l'Arduino est éteint, l'heure continue grâce à cette pile.

### 3. Appel de update()

```cpp
void loop() {
  rtc.update(); // ← OBLIGATOIRE pour alarmes et callbacks

  // Ton code...
}
```

### 4. Jour de la semaine

Le DS1302 utilise : 1=Dimanche, 2=Lundi, ..., 7=Samedi
