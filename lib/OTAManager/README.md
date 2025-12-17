# OTAManager Library

Librairie simple et complète pour gérer les mises à jour **Over-The-Air (OTA)** sur ESP8266 et ESP32. S'intègre parfaitement avec **WiFiManager**.

## ✨ Caractéristiques

- ✅ **Compatible ESP8266 et ESP32** - Détection automatique
- ✅ **Configuration simple** - 3 lignes de code suffisent
- ✅ **Callbacks personnalisables** - Hooks pour tous les événements
- ✅ **Gestion d'état** - Suivi de la progression et des erreurs
- ✅ **Sécurisé** - Support du mot de passe
- ✅ **mDNS intégré** - Recherche par nom (hostname.local)
- ✅ **Compatible WiFiManager** - Intégration parfaite

## 📦 Installation

```
lib/OTAManager/
├── OTAManager.h
├── OTAManager.cpp
└── examples/
    ├── BasicOTA/BasicOTA.ino
    ├── CompleteOTA/CompleteOTA.ino
    └── AdvancedOTA/AdvancedOTA.ino
```

## 🚀 Utilisation rapide

### Version ultra simple (comme ton code)

```cpp
#include <OTAManager.h>

OTAManager ota("ESP-Test");

void setup() {
  // Connecter WiFi d'abord
  WiFi.begin("SSID", "Password");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // Initialiser OTA
  ota.begin();
}

void loop() {
  ota.handle(); // C'est tout !
}
```

### Avec WiFiManager (recommandé)

```cpp
#include <WiFiManager.h>
#include <OTAManager.h>

WiFiManager wifi("SSID", "Password", "ESP-Device");
OTAManager ota("ESP-Device", "admin123"); // Avec mot de passe

void setup() {
  wifi.begin();
  wifi.connect();

  ota.begin();
}

void loop() {
  wifi.checkConnection();
  ota.handle();
}
```

## 📚 API Complète

### Constructeur

```cpp
OTAManager(hostname, password, port);
```

**Paramètres :**

- `hostname` : Nom de l'appareil (défaut: "ESP-OTA")
- `password` : Mot de passe OTA (défaut: aucun)
- `port` : Port OTA (défaut: 3232)

**Exemples :**

```cpp
OTAManager ota("ESP-Test");                    // Sans mot de passe
OTAManager ota("ESP-Test", "admin123");        // Avec mot de passe
OTAManager ota("ESP-Test", "admin", 8266);     // Avec port personnalisé
```

### Initialisation

```cpp
bool begin();              // Initialiser l'OTA
void end();                // Arrêter l'OTA
```

**Exemple :**

```cpp
if (ota.begin()) {
  Serial.println("OTA prêt !");
}
```

### Contrôle

```cpp
void enable();             // Activer
void disable();            // Désactiver
bool isEnabled();          // Vérifier l'état
void handle();             // À appeler dans loop()
```

**Exemple :**

```cpp
void loop() {
  ota.handle(); // Obligatoire !

  // Ton code...
}
```

### Configuration

```cpp
void setHostname(name);    // Changer le hostname
void setPassword(pwd);     // Changer le mot de passe
void setPort(port);        // Changer le port
```

### Callbacks

```cpp
void onStart(callback);              // Début de mise à jour
void onEnd(callback);                // Fin de mise à jour
void onProgress(progress, total);    // Progression
void onError(error);                 // Erreur
```

**Exemple complet :**

```cpp
ota.onStart([]() {
  Serial.println("Début MAJ !");
  // Désactiver d'autres services
});

ota.onEnd([]() {
  Serial.println("Fin MAJ !");
});

ota.onProgress([](unsigned int progress, unsigned int total) {
  int percent = (progress * 100) / total;
  Serial.printf("Progression: %d%%\n", percent);
});

ota.onError([](int error) {
  Serial.printf("Erreur: %d\n", error);
});
```

### Informations

```cpp
OTAState getState();              // État actuel
String getStateString();          // État en texte
const char* getHostname();        // Nom d'hôte
uint16_t getPort();               // Port
bool isUpdating();                // En cours de MAJ ?
```

**États possibles :**

- `OTA_IDLE` : Inactif
- `OTA_READY` : Prêt
- `OTA_UPDATING` : Mise à jour en cours
- `OTA_SUCCESS` : Succès
- `OTA_ERROR` : Erreur

**Exemple :**

```cpp
if (ota.isUpdating()) {
  Serial.println("Mise à jour en cours...");
}

Serial.println(ota.getStateString()); // "Prêt"
```

### Progression

```cpp
unsigned int getProgress();       // Octets téléchargés
unsigned int getProgressPercent(); // Pourcentage (0-100)
```

**Exemple :**

```cpp
if (ota.isUpdating()) {
  Serial.print("Progression: ");
  Serial.print(ota.getProgressPercent());
  Serial.println("%");
}
```

### Erreurs

```cpp
int getLastError();               // Code d'erreur
String getLastErrorString();      // Erreur en texte
```

**Codes d'erreur :**

- `OTA_AUTH_ERROR` : Mot de passe incorrect
- `OTA_BEGIN_ERROR` : Erreur de démarrage
- `OTA_CONNECT_ERROR` : Erreur de connexion
- `OTA_RECEIVE_ERROR` : Erreur de réception
- `OTA_END_ERROR` : Erreur de fin

### Utilitaires

```cpp
void printInfo();                 // Afficher les infos
String getMDNSUrl();              // URL mDNS (hostname.local)
```

## 💡 Exemples pratiques

### Ton code original simplifié

**Avant (ton code) :**

```cpp
// 80+ lignes de code dans main.cpp
ArduinoOTA.setPort(3232);
ArduinoOTA.setHostname("ESP-OTA");
ArduinoOTA.onStart([]() { /* ... */ });
ArduinoOTA.onEnd([]() { /* ... */ });
ArduinoOTA.onProgress([]() { /* ... */ });
ArduinoOTA.onError([]() { /* ... */ });
ArduinoOTA.begin();

void loop() {
  ArduinoOTA.handle();
  // LED blinking code...
}
```

**Après (avec OTAManager) :**

```cpp
#include <OTAManager.h>

OTAManager ota("ESP-OTA");

void setup() {
  WiFi.begin("SSID", "Password");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  ota.begin();
}

void loop() {
  ota.handle();
  // Ton code ici (sans la LED)
}
```

**Économie : 60+ lignes de code ! 🎉**

### Avec affichage de progression

```cpp
OTAManager ota("ESP-Device");

void setup() {
  // WiFi...
  ota.begin();

  // Callback de progression avec barre
  ota.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress * 100) / total;

    // Barre de progression ASCII
    Serial.print("[");
    for (int i = 0; i < 50; i++) {
      if (i < (percent / 2)) Serial.print("=");
      else if (i == (percent / 2)) Serial.print(">");
      else Serial.print(" ");
    }
    Serial.print("] ");
    Serial.print(percent);
    Serial.println("%");
  });
}
```

### Avec serveur web de monitoring

```cpp
WiFiManager wifi("SSID", "Password");
OTAManager ota("ESP-Monitor");

void setup() {
  wifi.begin();
  wifi.connect();
  ota.begin();

  wifi.startWebServer(80);

  // API pour récupérer l'état OTA
  wifi.on("/ota-status", []() {
    auto server = wifi.getServer();

    String json = "{";
    json += "\"state\":\"" + ota.getStateString() + "\",";
    json += "\"progress\":" + String(ota.getProgressPercent()) + ",";
    json += "\"updating\":" + String(ota.isUpdating() ? "true" : "false");
    json += "}";

    server->send(200, "application/json", json);
  });
}

void loop() {
  wifi.checkConnection();
  wifi.handleClient();
  ota.handle();
}
```

### Désactiver OTA temporairement

```cpp
OTAManager ota("ESP-Device");

void setup() {
  // WiFi...
  ota.begin();
}

void loop() {
  ota.handle();

  // Désactiver pendant une tâche critique
  if (criticalTask) {
    ota.disable();
    performCriticalTask();
    ota.enable();
  }
}
```

## 🔧 Configuration PlatformIO

### platformio.ini

```ini
[env:esp8266]
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps =
    OTAManager
    WiFiManager

upload_protocol = espota
upload_port = ESP-Device.local  # Ton hostname
upload_flags =
    --auth=admin123  # Ton mot de passe
```

## 📡 Utilisation de l'OTA

### Via Arduino IDE

1. **Tools → Port** : Sélectionner "ESP-Device at 192.168.x.x"
2. **Upload** comme d'habitude
3. Si mot de passe : entrer lors de l'upload

### Via PlatformIO

```bash
pio run -t upload --upload-port ESP-Device.local
```

### Via ligne de commande

```bash
python ~/.platformio/packages/framework-arduinoespressif8266/tools/espota.py \
  -i ESP-Device.local \
  -p 3232 \
  -a admin123 \
  -f .pio/build/esp8266/firmware.bin
```

## ⚠️ Bonnes pratiques

### 1. Toujours vérifier WiFi avant OTA

```cpp
if (WiFi.status() != WL_CONNECTED) {
  Serial.println("WiFi requis pour OTA !");
  return;
}
ota.begin();
```

### 2. Utiliser un mot de passe

```cpp
// ❌ MAL : Pas de sécurité
OTAManager ota("ESP-Device");

// ✅ BIEN : Avec mot de passe
OTAManager ota("ESP-Device", "motdepasse123");
```

### 3. Désactiver pendant les tâches critiques

```cpp
ota.onStart([]() {
  // Désactiver autres services
  stopMotors();
  closeSensors();
});
```

### 4. Gérer les erreurs

```cpp
ota.onError([](int error) {
  Serial.print("Erreur OTA: ");
  Serial.println(error);
  // Redémarrer si échec
  if (error == OTA_END_ERROR) {
    delay(1000);
    ESP.restart();
  }
});
```

### 5. Tester régulièrement

- Faire des uploads OTA fréquemment
- Vérifier que le mot de passe fonctionne
- Tester avec différentes tailles de firmware

## 🐛 Dépannage

**OTA non trouvé ?**

- Vérifier que WiFi est connecté
- Vérifier que `ota.begin()` a été appelé
- Ping `hostname.local` pour tester mDNS
- Vérifier le port (défaut: 3232)

**Erreur de mot de passe ?**

- Vérifier que le mot de passe est correct
- Utiliser `setPassword()` avant `begin()`
- Vérifier dans PlatformIO `upload_flags`

**Upload échoue à mi-chemin ?**

- Vérifier la qualité du signal WiFi
- Augmenter le timeout dans PlatformIO
- Vérifier l'espace flash disponible

**mDNS ne fonctionne pas ?**

- Windows : installer Bonjour Print Services
- Linux : installer avahi-daemon
- macOS : supporté nativement

## 📄 Licence

Libre d'utilisation pour projets personnels et commerciaux.
