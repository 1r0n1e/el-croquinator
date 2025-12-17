# WiFiManager Library

Librairie complète et facile d'utilisation pour gérer le WiFi sur **ESP8266** et **ESP32**. Compatible avec les deux plateformes avec détection automatique !

## 🌟 Caractéristiques

### Connexion WiFi

- ✅ Connexion automatique avec timeout configurable
- ✅ Reconnexion automatique en cas de perte
- ✅ Tentatives multiples configurables
- ✅ Détection automatique ESP8266/ESP32
- ✅ Gestion des erreurs et des états
- ✅ Mode Access Point de secours

### Serveur Web

- ✅ Serveur HTTP intégré
- ✅ Routes personnalisables (GET, POST, etc.)
- ✅ Pages HTML/JSON
- ✅ Gestion des fichiers statiques
- ✅ Handler 404 personnalisable
- ✅ Pages de statut par défaut

### NTP (Network Time Protocol)

- ✅ Synchronisation automatique de l'heure
- ✅ Support des fuseaux horaires
- ✅ Formats de date/heure personnalisables
- ✅ Timestamp Unix

### Informations réseau

- ✅ Adresse IP, MAC, hostname
- ✅ RSSI et qualité du signal
- ✅ État de connexion
- ✅ Scan des réseaux WiFi
- ✅ Export JSON

### Callbacks et événements

- ✅ Notification des changements d'état
- ✅ Gestion asynchrone

## 📦 Installation

### PlatformIO

Placez les fichiers dans votre projet :

```
lib/WiFiManager/
├── WiFiManager.h
├── WiFiManager.cpp
└── examples/
    ├── BasicUsage/BasicUsage.ino
    ├── WebServer/WebServer.ino
    ├── NTPTime/NTPTime.ino
    └── CompleteExample/CompleteExample.ino
```

La librairie détecte automatiquement si vous utilisez ESP8266 ou ESP32 !

## 🚀 Utilisation rapide

### Connexion WiFi simple

```cpp
#include <WiFiManager.h>

WiFiManager wifi("Mon_SSID", "Mon_Password", "ESP-Device");

void setup() {
  Serial.begin(115200);

  wifi.begin();

  if (wifi.connect()) {
    Serial.println("✓ Connecté !");
    Serial.print("IP: ");
    Serial.println(wifi.getIP());
  }
}

void loop() {
  wifi.checkConnection(); // Reconnexion auto si perdu
}
```

### Serveur Web avec LED RGB

```cpp
#include <WiFiManager.h>

WiFiManager wifi("SSID", "Password");
const int RED_PIN = 16;

void setup() {
  pinMode(RED_PIN, OUTPUT);

  wifi.begin();
  wifi.connect();
  wifi.startWebServer(80);

  // Page HTML
  wifi.serveStatic("/", "text/html", "<h1>Hello ESP!</h1>");

  // Route pour contrôler la LED
  wifi.on("/led", []() {
    int value = wifi.getServer()->arg("value").toInt();
    analogWrite(RED_PIN, value);
    wifi.getServer()->send(200, "text/plain", "OK");
  });
}

void loop() {
  wifi.checkConnection();
  wifi.handleClient();
}
```

### NTP - Récupération de l'heure

```cpp
#include <WiFiManager.h>

WiFiManager wifi("SSID", "Password");

void setup() {
  wifi.begin();
  wifi.connect();

  // Activer NTP (fuseau horaire Paris : UTC+1)
  wifi.enableNTP("pool.ntp.org", 3600, 3600);

  delay(2000); // Attendre la synchro

  Serial.println(wifi.getTime());      // "14:30:25"
  Serial.println(wifi.getDate());      // "16/12/2024"
  Serial.println(wifi.getDateTime());  // "16/12/2024 14:30:25"
}
```

## 📚 API Complète

### Constructeur

```cpp
WiFiManager(const char* ssid, const char* password, const char* hostname = "ESP-Device");
```

### Connexion WiFi

```cpp
bool begin();                              // Initialiser
bool connect();                            // Se connecter
bool isConnected();                        // Vérifier l'état
void disconnect();                         // Se déconnecter
void reconnect();                          // Reconnecter
void checkConnection();                    // Vérifier (dans loop)
void setConnectionTimeout(unsigned long ms);
void setMaxReconnectAttempts(uint8_t attempts);
void enableAutoReconnect(bool enable);
```

**Exemple :**

```cpp
wifi.setConnectionTimeout(30000);      // 30 secondes max
wifi.setMaxReconnectAttempts(5);       // 5 tentatives
wifi.enableAutoReconnect(true);
```

### Informations WiFi

```cpp
String getIP();                   // Adresse IP
String getMAC();                  // Adresse MAC
int getRSSI();                    // Force du signal (dBm)
String getSSID();                 // Nom du réseau
String getHostname();             // Nom de l'appareil
WiFiState getState();             // État de connexion
String getStateString();          // État en texte
String getSignalQuality();        // "Excellent", "Bon", etc.
uint8_t getSignalPercent();       // Force en %
void printDebugInfo();            // Afficher toutes les infos
```

**Exemple :**

```cpp
Serial.print("IP: "); Serial.println(wifi.getIP());
Serial.print("RSSI: "); Serial.println(wifi.getRSSI());
Serial.print("Signal: "); Serial.println(wifi.getSignalQuality());
```

### Serveur Web

```cpp
bool startWebServer(uint16_t port = 80);
void stopWebServer();
void handleClient();                      // À appeler dans loop
WebServerType* getServer();               // Accès direct au serveur
```

#### Routes HTTP

```cpp
void on(const char* uri, WebHandler handler);
void on(const char* uri, HTTPMethod method, WebHandler handler);
void onNotFound(WebHandler handler);
void serveStatic(const char* uri, const char* contentType, const char* content);
```

**Exemple :**

```cpp
// Route simple
wifi.on("/hello", []() {
  wifi.getServer()->send(200, "text/plain", "Hello World!");
});

// Route avec paramètres
wifi.on("/led", []() {
  auto server = wifi.getServer();
  if (server->hasArg("state")) {
    String state = server->arg("state");
    digitalWrite(LED_PIN, state == "on" ? HIGH : LOW);
    server->send(200, "text/plain", "OK");
  }
});

// Page HTML statique
wifi.serveStatic("/", "text/html", "<h1>Ma page</h1>");

// Page 404
wifi.onNotFound([]() {
  wifi.getServer()->send(404, "text/plain", "Page non trouvée");
});
```

#### Pages par défaut

```cpp
void enableDefaultPages(bool enable = true);
String getStatusJSON();
String getStatusHTML();
```

Active automatiquement :

- `/status` : Statut WiFi en JSON
- `/info` : Page HTML avec toutes les infos

**Exemple :**

```cpp
wifi.enableDefaultPages(true);
// Accéder à http://[IP]/status ou http://[IP]/info
```

### NTP (Heure réseau)

```cpp
void enableNTP(const char* server = "pool.ntp.org", long gmtOffset = 0, int dstOffset = 0);
void disableNTP();
bool syncTime();
String getTime(const char* format = "%H:%M:%S");
String getDate(const char* format = "%d/%m/%Y");
String getDateTime(const char* format = "%d/%m/%Y %H:%M:%S");
time_t getTimestamp();
```

**Fuseaux horaires (gmtOffset en secondes) :**

- Paris (UTC+1) : `3600`
- Londres (UTC+0) : `0`
- New York (UTC-5) : `-18000`
- Tokyo (UTC+9) : `32400`

**Exemple :**

```cpp
// Paris avec heure d'été
wifi.enableNTP("pool.ntp.org", 3600, 3600);

// Attendre la synchro
delay(2000);

// Afficher l'heure
Serial.println(wifi.getTime());              // "14:30:25"
Serial.println(wifi.getDate());              // "16/12/2024"
Serial.println(wifi.getTime("%H:%M"));       // "14:30"
Serial.println(wifi.getDate("%d %B %Y"));    // "16 décembre 2024"
```

### Mode Access Point

```cpp
bool startAP(const char* apSSID, const char* apPassword = NULL);
void stopAP();
```

**Exemple :**

```cpp
// Démarrer un point d'accès de secours
if (!wifi.connect()) {
  wifi.startAP("ESP-Setup", "12345678");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}
```

### Scan des réseaux

```cpp
int scanNetworks();
String getScannedNetwork(int index);
String getScannedNetworkJSON();
```

**Exemple :**

```cpp
int n = wifi.scanNetworks();
for (int i = 0; i < n; i++) {
  Serial.println(wifi.getScannedNetwork(i));
}

// Ou en JSON
Serial.println(wifi.getScannedNetworkJSON());
```

### Callbacks

```cpp
void setStateChangeCallback(WiFiEventCallback callback);
```

**Exemple :**

```cpp
void onWiFiChange(WiFiState state) {
  Serial.print("État: ");
  Serial.println(wifi.getStateString());
}

wifi.setStateChangeCallback(onWiFiChange);
```

## 🎯 Exemples complets

### Dashboard avec tout combiné

```cpp
#include <WiFiManager.h>

WiFiManager wifi("SSID", "Password", "ESP-Dashboard");

void setup() {
  Serial.begin(115200);

  wifi.begin();
  wifi.connect();
  wifi.enableNTP("pool.ntp.org", 3600, 3600);
  wifi.startWebServer(80);

  // Page d'accueil
  wifi.serveStatic("/", "text/html", R"(
    <!DOCTYPE html>
    <html>
    <body>
      <h1>Dashboard ESP</h1>
      <div id="info"></div>
      <script>
        fetch('/status')
          .then(r => r.json())
          .then(data => {
            document.getElementById('info').innerHTML =
              'IP: ' + data.ip + '<br>' +
              'RSSI: ' + data.rssi + ' dBm<br>' +
              'Heure: ' + data.time;
          });
      </script>
    </body>
    </html>
  )");

  wifi.enableDefaultPages(true);
}

void loop() {
  wifi.checkConnection();
  wifi.handleClient();
}
```

## 🔧 Configuration matérielle

### ESP8266 (NodeMCU, Wemos D1)

- Utilise `ESP8266WiFi` et `ESP8266WebServer`
- PWM 10-bit (0-1023)
- Broches recommandées : D1-D8

### ESP32 (DevKit, WROVER)

- Utilise `WiFi` et `WebServer`
- PWM 8-bit (0-255) par défaut
- Broches recommandées : GPIO 2, 4, 5, 16, 17, 18, 19

La librairie détecte automatiquement votre plateforme !

## ⚠️ Notes importantes

1. **Appels dans loop()** : N'oubliez pas d'appeler ces fonctions dans `loop()` :

   ```cpp
   wifi.checkConnection();  // Reconnexion auto
   wifi.handleClient();     // Gestion des requêtes web
   ```

2. **Timeout de connexion** : Ajustez selon votre réseau :

   ```cpp
   wifi.setConnectionTimeout(20000); // 20 secondes
   ```

3. **NTP** : Attendez 2-3 secondes après `enableNTP()` pour la synchro :

   ```cpp
   wifi.enableNTP();
   delay(2000);
   Serial.println(wifi.getTime());
   ```

4. **Pages PROGMEM** : Pour les grandes pages HTML, utilisez `PROGMEM` :

   ```cpp
   const char HTML[] PROGMEM = R"rawliteral(
     <!-- votre HTML -->
   )rawliteral";
   ```

5. **Sécurité** : Ne stockez pas vos identifiants WiFi en clair dans le code en production !

## 🐛 Dépannage

**WiFi ne se connecte pas ?**

- Vérifiez le SSID et le mot de passe
- Augmentez le timeout : `wifi.setConnectionTimeout(30000)`
- Vérifiez la portée du signal

**Serveur web ne répond pas ?**

- Vérifiez que `wifi.handleClient()` est dans `loop()`
- Vérifiez l'IP avec `Serial.println(wifi.getIP())`
- Testez avec `/status` ou `/info`

**Heure NTP incorrecte ?**

- Vérifiez le fuseau horaire (gmtOffset)
- Attendez quelques secondes après `enableNTP()`
- Testez avec un autre serveur NTP

## 📄 Licence

Libre d'utilisation pour vos projets personnels et commerciaux.
