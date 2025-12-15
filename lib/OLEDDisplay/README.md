# OLEDDisplay Library

Librairie complète et polyvalente pour gérer les écrans OLED SSD1306 (128x64 ou 128x32) avec de nombreuses fonctionnalités prêtes à l'emploi.

## 📦 Dépendances

Cette librairie nécessite :

- **Adafruit_SSD1306** (>= 2.5.0)
- **Adafruit_GFX** (>= 1.11.0)

Installation via PlatformIO :

```ini
lib_deps =
    adafruit/Adafruit SSD1306 @ ^2.5.0
    adafruit/Adafruit GFX Library @ ^1.11.0
```

## ✨ Caractéristiques

### Fonctionnalités de base

- ✅ Affichage de texte simple, aligné, et centré
- ✅ Support de différentes tailles de texte (1-8)
- ✅ Gestion automatique du rafraîchissement
- ✅ Timer d'affichage automatique

### Affichages avancés

- ✅ Messages avec titre
- ✅ Textes longs avec wrapping automatique
- ✅ Images/bitmaps (centrées ou positionnées)
- ✅ Barres de progression avec pourcentage
- ✅ Affichage de l'heure et de la date
- ✅ Valeurs numériques avec labels et unités

### Indicateurs et graphiques

- ✅ Indicateur de batterie
- ✅ Signal WiFi (0-4 barres)
- ✅ Formes géométriques (boîtes, lignes, cercles)
- ✅ Menu simple avec sélection

### Combinaisons prédéfinies

- ✅ Image + barre de progression
- ✅ Image + texte
- ✅ Dashboard multi-informations

## 📁 Installation

```
lib/OLEDDisplay/
├── OLEDDisplay.h
├── OLEDDisplay.cpp
├── README.md
└── examples/
    ├── BasicUsage/BasicUsage.ino
    ├── AdvancedUsage/AdvancedUsage.ino
    └── CombinedFeatures/CombinedFeatures.ino
```

## 🚀 Utilisation rapide

```cpp
#include <OLEDDisplay.h>

// Créer l'instance (largeur, hauteur, adresse I2C)
OLEDDisplay oled(128, 64, 0x3C);

void setup() {
  // Initialiser
  if (!oled.begin()) {
    Serial.println("Erreur OLED !");
    while(1);
  }

  // Afficher un message (3 secondes)
  oled.printMessage("Hello", "World!", 3);
}

void loop() {
  // Gérer le timer automatique
  oled.update();
}
```

## 📚 API Complète

### Initialisation

```cpp
OLEDDisplay oled(uint8_t width = 128, uint8_t height = 64, uint8_t address = 0x3C);
bool begin(int8_t resetPin = -1);
```

**Exemple :**

```cpp
OLEDDisplay oled(128, 64, 0x3C); // Écran 128x64, adresse 0x3C
oled.begin();
```

### Configuration

```cpp
void setAutoRefresh(bool enabled);    // Active/désactive le rafraîchissement auto
void setBrightness(uint8_t brightness); // Luminosité 0-255
```

### Gestion du timer

```cpp
void startTimer(unsigned int seconds); // Démarre un timer d'affichage
void stopTimer();                      // Arrête le timer
bool isTimerActive();                  // Vérifie si le timer est actif
void update();                         // À appeler dans loop()
```

**Exemple :**

```cpp
oled.printMessage("Info", "Message temporaire", 5); // Affiche 5 secondes
// Le message disparaîtra automatiquement si vous appelez oled.update() dans loop()
```

### Affichage de texte

```cpp
// Texte simple
void printText(const char* text, uint8_t x, uint8_t y, uint8_t size = 1);

// Texte aligné (ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT)
void printTextAligned(const char* text, TextAlign align, uint8_t y, uint8_t size = 1);

// Texte centré (vertical et horizontal)
void printTextCentered(const char* text, uint8_t size = 2);

// Message avec titre
void printMessage(const char* title, const char* message, unsigned int displayTimeSec = 3);

// Texte long (wrapping automatique)
void printLongText(const char* text, uint8_t size = 1, unsigned int displayTimeSec = 5);
```

**Exemples :**

```cpp
oled.printText("Hello", 0, 0, 1);                    // Coin supérieur gauche
oled.printTextAligned("Centre", ALIGN_CENTER, 20, 2); // Centré horizontalement
oled.printTextCentered("MILIEU", 3);                  // Centré partout
```

### Images

```cpp
// Image à une position
void printImage(const uint8_t* bitmap, uint8_t width, uint8_t height,
                uint8_t x, uint8_t y);

// Image centrée
void printImageCentered(const uint8_t* bitmap, uint8_t width, uint8_t height);
```

**Exemple :**

```cpp
const unsigned char MY_IMAGE[] PROGMEM = { /* données */ };
oled.printImageCentered(MY_IMAGE, 60, 40);
```

💡 **Conseil :** Utilisez [image2cpp](https://javl.github.io/image2cpp/) pour convertir vos images en tableaux.

### Barres de progression

```cpp
// Barre personnalisée
void drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                     uint8_t progress, bool showPercentage = true);

// Barre en bas de l'écran
void drawProgressBarBottom(uint8_t progress, bool showPercentage = true);
```

**Exemple :**

```cpp
for (int i = 0; i <= 100; i += 10) {
  oled.clear();
  oled.drawProgressBarBottom(i, true);
  oled.refresh();
  delay(200);
}
```

### Heure et date

```cpp
void printTime(uint8_t hours, uint8_t minutes, uint8_t seconds,
               TextAlign align = ALIGN_CENTER, uint8_t y = 0);

void printDate(uint8_t day, uint8_t month, uint16_t year,
               TextAlign align = ALIGN_CENTER, uint8_t y = 0);
```

**Exemple :**

```cpp
oled.printTime(14, 30, 45, ALIGN_CENTER, 20);  // 14:30:45
oled.printDate(15, 12, 2024, ALIGN_CENTER, 40); // 15/12/2024
```

### Valeurs avec unités

```cpp
void printValue(const char* label, float value, uint8_t decimals = 1,
                const char* unit = "", uint8_t y = 0);
```

**Exemple :**

```cpp
oled.printValue("Temp", 23.5, 1, "C", 0);    // Temp: 23.5 °C
oled.printValue("Humid", 65.0, 0, "%", 25);  // Humid: 65 %
```

### Indicateurs

```cpp
void drawBattery(uint8_t x, uint8_t y, uint8_t percentage);
void drawWifiSignal(uint8_t x, uint8_t y, uint8_t strength); // 0-4
```

**Exemple :**

```cpp
oled.drawBattery(100, 0, 75);   // Batterie à 75%
oled.drawWifiSignal(100, 20, 3); // Signal WiFi 3/4 barres
```

### Formes graphiques

```cpp
void drawBox(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled = false);
void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void drawCircle(uint8_t x, uint8_t y, uint8_t radius, bool filled = false);
```

### Combinaisons prédéfinies

```cpp
// Image avec barre de progression en bas
void printImageWithProgress(const uint8_t* bitmap, uint8_t imgWidth,
                            uint8_t imgHeight, uint8_t progress);

// Image avec texte en dessous
void printImageWithText(const uint8_t* bitmap, uint8_t imgWidth,
                        uint8_t imgHeight, const char* text, uint8_t textSize = 1);
```

**Exemple :**

```cpp
// Affichage de progression avec une image
for (int i = 0; i <= 100; i += 5) {
  oled.printImageWithProgress(MY_IMAGE, 60, 40, i);
  delay(100);
}
```

### Menu

```cpp
void printMenuItem(const char* text, uint8_t index, uint8_t totalItems, bool selected);
```

**Exemple :**

```cpp
const char* items[] = {"Option 1", "Option 2", "Option 3"};
oled.clear();
for (int i = 0; i < 3; i++) {
  oled.printMenuItem(items[i], i, 3, i == selectedIndex);
}
oled.refresh();
```

### Contrôle avancé

```cpp
void clear();                              // Efface le buffer
void clearAndDisplay();                    // Efface et affiche
void refresh();                            // Rafraîchit l'écran
Adafruit_SSD1306* getDisplay();           // Accès à l'objet Adafruit
```

## 💡 Exemples pratiques

### Dashboard complet

```cpp
void displayDashboard() {
  oled.clear();
  oled.setAutoRefresh(false); // Optimisation

  // En-tête
  oled.printText("Dashboard", 0, 0, 1);
  oled.drawBattery(90, 0, batteryLevel);

  // Séparateur
  oled.drawLine(0, 10, 128, 10);

  // Données
  oled.printValue("Temp", 23.5, 1, "C", 15);
  oled.printValue("Humid", 65.0, 0, "%", 35);

  // Progression
  oled.drawProgressBarBottom(downloadProgress, true);

  oled.setAutoRefresh(true);
  oled.refresh();
}
```

### Station météo

```cpp
oled.clear();
oled.printTextAligned("METEO", ALIGN_CENTER, 0, 1);
oled.drawBox(5, 12, 118, 40, false);

char buffer[16];
sprintf(buffer, "%.1f", temperature);
oled.printTextCentered(buffer, 3);
oled.printText("Celsius", 40, 40, 1);
oled.refresh();
```

### Écran de chargement

```cpp
for (int i = 0; i <= 100; i += 2) {
  oled.printImageWithProgress(LOGO, 60, 40, i);
  delay(50);
}
```

## 🔧 Configuration matérielle

### Connexion I2C typique (ESP8266/ESP32)

| OLED | ESP8266    | ESP32  |
| ---- | ---------- | ------ |
| VCC  | 3.3V       | 3.3V   |
| GND  | GND        | GND    |
| SDA  | D2 (GPIO4) | GPIO21 |
| SCL  | D1 (GPIO5) | GPIO22 |

### Trouver l'adresse I2C

```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);

  byte error, address;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Adresse trouvée: 0x");
      Serial.println(address, HEX);
    }
  }
}
```

## ⚠️ Conseils et bonnes pratiques

1. **Optimisation du rafraîchissement** : Désactivez `autoRefresh` lors de multiples opérations, puis appelez `refresh()` à la fin.

```cpp
oled.setAutoRefresh(false);
oled.printText("Ligne 1", 0, 0, 1);
oled.printText("Ligne 2", 0, 10, 1);
oled.printText("Ligne 3", 0, 20, 1);
oled.setAutoRefresh(true);
oled.refresh(); // Un seul rafraîchissement
```

2. **Gestion de la mémoire** : Les images PROGMEM économisent la RAM.

```cpp
const unsigned char IMAGE[] PROGMEM = { /* ... */ };
```

3. **Timer automatique** : N'oubliez pas d'appeler `update()` dans `loop()` pour que les timers fonctionnent.

4. **Luminosité** : Ajustez selon l'environnement.

```cpp
oled.setBrightness(128); // Luminosité moyenne
```

## 🐛 Dépannage

**Écran vide ou ne s'initialise pas ?**

- Vérifiez l'adresse I2C (0x3C ou 0x3D)
- Vérifiez les connexions SDA/SCL
- Testez la tension d'alimentation (3.3V stable)

**Affichage clignotant ?**

- Désactivez `autoRefresh` pendant les mises à jour multiples
- Appelez `refresh()` seulement quand nécessaire

**Texte tronqué ?**

- Vérifiez les limites de l'écran (128x64)
- Utilisez `printLongText()` pour les textes longs

## 📄 Licence

Libre d'utilisation pour vos projets personnels et commerciaux.
