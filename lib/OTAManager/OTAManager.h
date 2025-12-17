/*
 * OTAManager.h
 * Gestion des mises à jour Over-The-Air (OTA) pour ESP8266 et ESP32
 * Compatible avec WiFiManager
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>

// Détection automatique de la plateforme
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ArduinoOTA.h>
#else
#error "Cette librairie nécessite ESP8266 ou ESP32"
#endif

// État de l'OTA
enum class OTAState
{
    OTA_IDLE,     // Inactif
    OTA_READY,    // Prêt
    OTA_UPDATING, // Mise à jour en cours
    OTA_SUCCESS,  // Succès
    OTA_ERROR     // Erreur
};

// Type de mise à jour
enum OTAUpdateType
{
    OTA_UPDATE_SKETCH,    // Programme
    OTA_UPDATE_FILESYSTEM // Système de fichiers (SPIFFS/LittleFS)
};

// Callbacks
typedef void (*OTACallback)();
typedef void (*OTAProgressCallback)(unsigned int progress, unsigned int total);
typedef void (*OTAErrorCallback)(int error);

class OTAManager
{
private:
    // Configuration
    const char *hostname;
    const char *password;
    uint16_t port;

    // État
    OTAState currentState;
    OTAUpdateType updateType;
    bool initialized;
    bool enabled;

    // Statistiques
    unsigned int lastProgress;
    unsigned int lastTotal;
    int lastError;

    // Callbacks personnalisés
    OTACallback onStartCallback;
    OTACallback onEndCallback;
    OTAProgressCallback onProgressCallback;
    OTAErrorCallback onErrorCallback;

    // Méthodes internes
    void setupCallbacks();
    const char *getErrorString(int error) const;

public:
    // Constructeur
    OTAManager(const char *hostName = "ESP-OTA", const char *otaPassword = nullptr, uint16_t otaPort = 3232);

    // Initialisation
    bool begin();
    void end();

    // Contrôle
    void enable();
    void disable();
    bool isEnabled() const;
    void handle(); // À appeler dans loop()

    // Configuration
    void setHostname(const char *name);
    void setPassword(const char *pwd);
    void setPort(uint16_t portNumber);

    // Callbacks
    void onStart(OTACallback callback);
    void onEnd(OTACallback callback);
    void onProgress(OTAProgressCallback callback);
    void onError(OTAErrorCallback callback);

    // Informations
    OTAState getState() const;
    String getStateString() const;
    const char *getHostname() const;
    uint16_t getPort() const;
    bool isUpdating() const;

    // Progression
    unsigned int getProgress() const;
    unsigned int getProgressPercent() const;

    // Erreurs
    int getLastError() const;
    String getLastErrorString() const;

    // Utilitaires
    void printInfo();
    String getMDNSUrl() const;
};

#endif // OTA_MANAGER_H