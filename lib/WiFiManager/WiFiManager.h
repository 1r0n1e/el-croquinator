/*
 * WiFiManager.h
 * Librairie complète pour gérer le WiFi sur ESP8266 et ESP32
 * Connexion, serveur web, NTP, gestion des erreurs
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Détection automatique de la plateforme
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#define WebServerType ESP8266WebServer
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#define WebServerType WebServer
#else
#error "Cette librairie nécessite ESP8266 ou ESP32"
#endif

#include <WiFiUdp.h>
#include <time.h>
#include "HomePage.h" // default Home Page

// États de connexion
enum WifiState
{
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_CONNECTION_FAILED,
    WIFI_CONNECTION_LOST
};

// Type de callback pour les événements
typedef void (*WiFiEventCallback)(WifiState state);
typedef void (*WebHandler)(WebServerType &server);

class WiFiManager
{
private:
    // Configuration WiFi
    const char *ssid;
    const char *password;
    const char *hostname;

    // Serveur Web
    WebServerType *webServer;
    bool serverEnabled;
    uint16_t serverPort;

    // État
    WifiState currentState;
    unsigned long startConnectTime; // Pour le timeout non bloquant
    bool isConnecting = false;      // Flag pour savoir si on attend une réponse
    unsigned long lastConnectionAttempt;
    unsigned long connectionTimeout;
    uint8_t reconnectAttempts;
    uint8_t maxReconnectAttempts;

    // Callbacks
    WiFiEventCallback onStateChange;

    // NTP (Network Time Protocol)
    bool ntpEnabled;
    const char *ntpServer;
    long gmtOffsetSec;
    int daylightOffsetSec;

    // Méthodes privées
    void updateState(WifiState newState);
    void printConnectionStatus();

public:
    // Constructeur
    WiFiManager(const char *ssid, const char *password, const char *hostname = "ESP-Device");

    // Initialisation
    bool begin();
    void setConnectionTimeout(unsigned long timeoutMs);
    void setMaxReconnectAttempts(uint8_t attempts);

    // Connexion WiFi
    bool connect();
    bool isConnected();
    void disconnect();
    void reconnect();
    void checkConnection(); // À appeler dans loop()

    // Informations WiFi
    String getIP();
    String getMAC();
    int getRSSI();
    String getSSID();
    String getHostname();
    WifiState getState();
    String getStateString();

    // Qualité du signal
    String getSignalQuality();
    uint8_t getSignalPercent();

    // Serveur Web
    bool startWebServer(uint16_t port = 80);
    void stopWebServer();
    void handleClient(); // À appeler dans loop()
    WebServerType *getServer();

    // Routes HTTP
    void on(const char *uri, WebHandler handler);
    void on(const char *uri, HTTPMethod method, WebHandler handler);
    void onNotFound(WebHandler handler);
    void serveStatic(const char *uri, const char *contentType, const char *content);

    // Pages par défaut
    void enableDefaultPages(bool enable = true);
    String getDefaultHTML();
    String getStatusJSON();
    String getStatusHTML();

    // NTP (récupération de l'heure)
    void enableNTP(const char *server = "pool.ntp.org", long gmtOffset = 0, int dstOffset = 0);
    void disableNTP();
    bool syncTime();
    String getTime(const char *format = "%H:%M:%S");
    String getDate(const char *format = "%d/%m/%Y");
    String getDateTime(const char *format = "%d/%m/%Y %H:%M:%S");
    time_t getTimestamp();

    // Mode Access Point (optionnel)
    bool startAP(const char *apSSID, const char *apPassword = NULL);
    void stopAP();

    // Callbacks
    void setStateChangeCallback(WiFiEventCallback callback);

    // Utilitaires
    void printDebugInfo();
    void enableAutoReconnect(bool enable = true);

    // Scan des réseaux
    int scanNetworks();
    String getScannedNetwork(int index);
    String getScannedNetworkJSON();
};

#endif // WIFI_MANAGER_H