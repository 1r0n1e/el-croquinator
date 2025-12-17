/*
 * WiFiManager.cpp
 * Implémentation de la librairie WiFiManager
 */

#include "WiFiManager.h"

// Constructeur
WiFiManager::WiFiManager(const char *ssid, const char *password, const char *hostname)
{
    this->ssid = ssid;
    this->password = password;
    this->hostname = hostname;

    webServer = nullptr;
    serverEnabled = false;
    serverPort = 80;

    currentState = WIFI_DISCONNECTED;
    lastConnectionAttempt = 0;
    connectionTimeout = 30000; // 30 secondes par défaut
    reconnectAttempts = 0;
    maxReconnectAttempts = 5;

    onStateChange = nullptr;

    ntpEnabled = false;
    ntpServer = "pool.ntp.org";
    gmtOffsetSec = 0;
    daylightOffsetSec = 0;
}

// Initialisation
bool WiFiManager::begin()
{
    Serial.println(F("[WiFi] Initialisation..."));

#ifdef ESP8266
    WiFi.mode(WIFI_STA);
    WiFi.hostname(hostname);
#else // ESP32
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname);
#endif

    return true;
}

void WiFiManager::setConnectionTimeout(unsigned long timeoutMs)
{
    connectionTimeout = timeoutMs;
}

void WiFiManager::setMaxReconnectAttempts(uint8_t attempts)
{
    maxReconnectAttempts = attempts;
}

// Connexion WiFi
bool WiFiManager::connect()
{
    Serial.println(F("[WiFi] Tentative de connexion..."));
    Serial.print(F("[WiFi] SSID: "));
    Serial.println(ssid);

    updateState(WIFI_CONNECTING);

    WiFi.begin(ssid, password);
    lastConnectionAttempt = millis();

    // Attente de la connexion
    while (WiFi.status() != WL_CONNECTED &&
           (millis() - lastConnectionAttempt < connectionTimeout))
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        updateState(WIFI_CONNECTED);
        reconnectAttempts = 0;
        printConnectionStatus();

        // Synchroniser l'heure si NTP est activé
        if (ntpEnabled)
        {
            syncTime();
        }

        return true;
    }
    else
    {
        updateState(WIFI_CONNECTION_FAILED);
        Serial.println(F("[WiFi] Échec de connexion"));
        return false;
    }
}

bool WiFiManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect()
{
    WiFi.disconnect();
    updateState(WIFI_DISCONNECTED);
    Serial.println(F("[WiFi] Déconnecté"));
}

void WiFiManager::reconnect()
{
    if (reconnectAttempts < maxReconnectAttempts)
    {
        reconnectAttempts++;
        Serial.print(F("[WiFi] Reconnexion (tentative "));
        Serial.print(reconnectAttempts);
        Serial.print("/");
        Serial.print(maxReconnectAttempts);
        Serial.println(")");
        connect();
    }
    else
    {
        Serial.println(F("[WiFi] Nombre maximum de tentatives atteint"));
        updateState(WIFI_CONNECTION_FAILED);
    }
}

void WiFiManager::checkConnection()
{
    static unsigned long lastCheck = 0;

    // Vérifier toutes les 10 secondes
    if (millis() - lastCheck > 10000)
    {
        lastCheck = millis();

        if (WiFi.status() != WL_CONNECTED && currentState == WIFI_CONNECTED)
        {
            updateState(WIFI_CONNECTION_LOST);
            Serial.println(F("[WiFi] Connexion perdue"));
            reconnect();
        }
    }
}

// Informations WiFi
String WiFiManager::getIP()
{
    return WiFi.localIP().toString();
}

String WiFiManager::getMAC()
{
    return WiFi.macAddress();
}

int WiFiManager::getRSSI()
{
    return WiFi.RSSI();
}

String WiFiManager::getSSID()
{
    return String(WiFi.SSID());
}

String WiFiManager::getHostname()
{
#ifdef ESP8266
    return String(WiFi.hostname());
#else
    return String(WiFi.getHostname());
#endif
}

WifiState WiFiManager::getState()
{
    return currentState;
}

String WiFiManager::getStateString()
{
    switch (currentState)
    {
    case WIFI_DISCONNECTED:
        return "Déconnecté";
    case WIFI_CONNECTING:
        return "Connexion...";
    case WIFI_CONNECTED:
        return "Connecté";
    case WIFI_CONNECTION_FAILED:
        return "Échec";
    case WIFI_CONNECTION_LOST:
        return "Perdu";
    default:
        return "Inconnu";
    }
}

// Qualité du signal
String WiFiManager::getSignalQuality()
{
    int rssi = getRSSI();
    if (rssi > -50)
        return "Excellent";
    if (rssi > -60)
        return "Bon";
    if (rssi > -70)
        return "Moyen";
    if (rssi > -80)
        return "Faible";
    return "Très faible";
}

uint8_t WiFiManager::getSignalPercent()
{
    int rssi = getRSSI();
    // Conversion RSSI (-100 à -50) en pourcentage (0 à 100)
    return constrain(map(rssi, -100, -50, 0, 100), 0, 100);
}

// Serveur Web
bool WiFiManager::startWebServer(uint16_t port)
{
    if (webServer != nullptr)
    {
        stopWebServer();
    }

    if (!isConnected())
    {
        Serial.println(F("[Web] Impossible de démarrer: WiFi non connecté"));
        return false;
    }

    serverPort = port;
    webServer = new WebServerType(serverPort);

    webServer->begin();
    serverEnabled = true;

    Serial.print(F("[Web] Serveur démarré sur http://"));
    Serial.print(getIP());
    Serial.print(":");
    Serial.println(serverPort);

    return true;
}

void WiFiManager::stopWebServer()
{
    if (webServer != nullptr)
    {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
    }
    serverEnabled = false;
    Serial.println(F("[Web] Serveur arrêté"));
}

void WiFiManager::handleClient()
{
    if (serverEnabled && webServer != nullptr)
    {
        webServer->handleClient();
    }
}

WebServerType *WiFiManager::getServer()
{
    return webServer;
}

// Routes HTTP
void WiFiManager::on(const char *uri, WebHandler handler)
{
    if (webServer != nullptr)
    {
        webServer->on(uri, [this, handler]()
                      { handler(*webServer); });
    }
}

void WiFiManager::on(const char *uri, HTTPMethod method, WebHandler handler)
{
    if (webServer != nullptr)
    {
        webServer->on(uri, method, [this, handler]()
                      { handler(*webServer); });
    }
}

void WiFiManager::onNotFound(WebHandler handler)
{
    if (webServer != nullptr)
    {
        webServer->onNotFound([this, handler]()
                              { handler(*webServer); });
    }
}

void WiFiManager::serveStatic(const char *uri, const char *contentType, const char *content)
{
    if (webServer != nullptr)
    {
        webServer->on(uri, [this, contentType, content]()
                      { webServer->send(200, contentType, content); });
    }
}

// Pages par défaut
void WiFiManager::enableDefaultPages(bool enable)
{
    if (!enable || webServer == nullptr)
        return;

    // Default Home Page
    webServer->on("/", [this]()
                  { webServer->send(200, "text/html", getDefaultHTML()); });

    // Page de statut JSON
    webServer->on("/status", [this]()
                  { webServer->send(200, "application/json", getStatusJSON()); });

    // Page de statut HTML
    webServer->on("/info", [this]()
                  { webServer->send(200, "text/html", getStatusHTML()); });
}

String WiFiManager::getDefaultHTML()
{
    return HomePage::getHTML();
}

String WiFiManager::getStatusJSON()
{
    String json = "{";
    json += "\"ssid\":\"" + getSSID() + "\",";
    json += "\"ip\":\"" + getIP() + "\",";
    json += "\"mac\":\"" + getMAC() + "\",";
    json += "\"rssi\":" + String(getRSSI()) + ",";
    json += "\"signal\":\"" + getSignalQuality() + "\",";
    json += "\"hostname\":\"" + getHostname() + "\",";
    json += "\"state\":\"" + getStateString() + "\"";

    if (ntpEnabled)
    {
        json += ",\"time\":\"" + getTime() + "\",";
        json += "\"date\":\"" + getDate() + "\"";
    }

    json += "}";
    return json;
}

String WiFiManager::getStatusHTML()
{
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'><title>WiFi Status</title>";
    html += "<style>body{font-family:Arial;margin:20px;}";
    html += "table{border-collapse:collapse;width:100%;max-width:600px;}";
    html += "td,th{border:1px solid #ddd;padding:8px;text-align:left;}";
    html += "th{background-color:#4CAF50;color:white;}</style></head><body>";
    html += "<h1>Statut WiFi</h1><table>";
    html += "<tr><th>Paramètre</th><th>Valeur</th></tr>";
    html += "<tr><td>État</td><td>" + getStateString() + "</td></tr>";
    html += "<tr><td>SSID</td><td>" + getSSID() + "</td></tr>";
    html += "<tr><td>IP</td><td>" + getIP() + "</td></tr>";
    html += "<tr><td>MAC</td><td>" + getMAC() + "</td></tr>";
    html += "<tr><td>Hostname</td><td>" + getHostname() + "</td></tr>";
    html += "<tr><td>RSSI</td><td>" + String(getRSSI()) + " dBm</td></tr>";
    html += "<tr><td>Signal</td><td>" + getSignalQuality() + " (" + String(getSignalPercent()) + "%)</td></tr>";

    if (ntpEnabled)
    {
        html += "<tr><td>Heure</td><td>" + getTime() + "</td></tr>";
        html += "<tr><td>Date</td><td>" + getDate() + "</td></tr>";
    }

    html += "</table></body></html>";
    return html;
}

// NTP
void WiFiManager::enableNTP(const char *server, long gmtOffset, int dstOffset)
{
    ntpEnabled = true;
    ntpServer = server;
    gmtOffsetSec = gmtOffset;
    daylightOffsetSec = dstOffset;

    if (isConnected())
    {
        syncTime();
    }
}

void WiFiManager::disableNTP()
{
    ntpEnabled = false;
}

bool WiFiManager::syncTime()
{
    if (!ntpEnabled || !isConnected())
        return false;

    Serial.print(F("[NTP] Synchronisation avec "));
    Serial.println(ntpServer);

    configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);

    // Attendre la synchronisation
    int attempts = 0;
    while (time(nullptr) < 100000 && attempts < 10)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (time(nullptr) > 100000)
    {
        Serial.println(F("[NTP] Synchronisation réussie"));
        Serial.print(F("[NTP] Heure actuelle: "));
        Serial.println(getDateTime());
        return true;
    }
    else
    {
        Serial.println(F("[NTP] Échec de synchronisation"));
        return false;
    }
}

String WiFiManager::getTime(const char *format)
{
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), format, timeinfo);
    return String(buffer);
}

String WiFiManager::getDate(const char *format)
{
    return getTime(format);
}

String WiFiManager::getDateTime(const char *format)
{
    return getTime(format);
}

time_t WiFiManager::getTimestamp()
{
    return time(nullptr);
}

// Mode Access Point
bool WiFiManager::startAP(const char *apSSID, const char *apPassword)
{
    Serial.print(F("[AP] Démarrage du point d'accès: "));
    Serial.println(apSSID);

    bool success;
    if (apPassword != NULL && strlen(apPassword) > 0)
    {
        success = WiFi.softAP(apSSID, apPassword);
    }
    else
    {
        success = WiFi.softAP(apSSID);
    }

    if (success)
    {
        Serial.print(F("[AP] IP: "));
        Serial.println(WiFi.softAPIP());
    }

    return success;
}

void WiFiManager::stopAP()
{
    WiFi.softAPdisconnect(true);
    Serial.println(F("[AP] Point d'accès arrêté"));
}

// Callbacks
void WiFiManager::setStateChangeCallback(WiFiEventCallback callback)
{
    onStateChange = callback;
}

// Utilitaires
void WiFiManager::printDebugInfo()
{
    Serial.println(F("\n===== WiFi Debug Info ====="));
    Serial.print(F("État: "));
    Serial.println(getStateString());
    Serial.print(F("SSID: "));
    Serial.println(getSSID());
    Serial.print(F("IP: "));
    Serial.println(getIP());
    Serial.print(F("MAC: "));
    Serial.println(getMAC());
    Serial.print(F("Hostname: "));
    Serial.println(getHostname());
    Serial.print(F("RSSI: "));
    Serial.print(getRSSI());
    Serial.println(" dBm");
    Serial.print(F("Signal: "));
    Serial.print(getSignalQuality());
    Serial.print(" (");
    Serial.print(getSignalPercent());
    Serial.println("%)");

    if (ntpEnabled)
    {
        Serial.print(F("Heure: "));
        Serial.println(getDateTime());
    }

    Serial.println(F("===========================\n"));
}

void WiFiManager::enableAutoReconnect(bool enable)
{
    WiFi.setAutoReconnect(enable);
}

// Scan des réseaux
int WiFiManager::scanNetworks()
{
    Serial.println(F("[WiFi] Scan des réseaux..."));
    int n = WiFi.scanNetworks();
    Serial.print(n);
    Serial.println(F(" réseau(x) trouvé(s)"));
    return n;
}

String WiFiManager::getScannedNetwork(int index)
{
    if (index < 0 || index >= WiFi.scanComplete())
        return "";

    String info = WiFi.SSID(index);
    info += " (";
    info += WiFi.RSSI(index);
    info += " dBm)";
    info += (WiFi.encryptionType(index) ==
#ifdef ESP8266
             ENC_TYPE_NONE
#else
             WIFI_AUTH_OPEN
#endif
             )
                ? " "
                : "*";

    return info;
}

String WiFiManager::getScannedNetworkJSON()
{
    int n = WiFi.scanComplete();
    String json = "{\"networks\":[";

    for (int i = 0; i < n; i++)
    {
        if (i > 0)
            json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encrypted\":" + String(WiFi.encryptionType(i) !=
#ifdef ESP8266
                                          ENC_TYPE_NONE
#else
                                          WIFI_AUTH_OPEN
#endif
                                          ) +
                "}";
    }

    json += "]}";
    return json;
}

// Méthodes privées
void WiFiManager::updateState(WifiState newState)
{
    if (currentState != newState)
    {
        currentState = newState;
        if (onStateChange != nullptr)
        {
            onStateChange(currentState);
        }
    }
}

void WiFiManager::printConnectionStatus()
{
    Serial.println(F("[WiFi] ✓ Connecté avec succès !"));
    Serial.print(F("[WiFi] SSID: "));
    Serial.println(getSSID());
    Serial.print(F("[WiFi] IP: "));
    Serial.println(getIP());
    Serial.print(F("[WiFi] MAC: "));
    Serial.println(getMAC());
    Serial.print(F("[WiFi] Hostname: "));
    Serial.println(getHostname());
    Serial.print(F("[WiFi] RSSI: "));
    Serial.print(getRSSI());
    Serial.print(" dBm (");
    Serial.print(getSignalQuality());
    Serial.println(")");
}