/*
 * OTAManager.cpp
 * Implémentation de la classe OTAManager
 */

#include "OTAManager.h"

// Constructeur
OTAManager::OTAManager(const char *hostName, const char *otaPassword, uint16_t otaPort)
{
    hostname = hostName;
    password = otaPassword;
    port = otaPort;

    currentState = OTAState::OTA_IDLE;
    updateType = OTA_UPDATE_SKETCH;
    initialized = false;
    enabled = false;

    lastProgress = 0;
    lastTotal = 0;
    lastError = 0;

    onStartCallback = nullptr;
    onEndCallback = nullptr;
    onProgressCallback = nullptr;
    onErrorCallback = nullptr;
}

// Initialisation
bool OTAManager::begin()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("[OTA] Erreur : WiFi non connecté !"));
        return false;
    }

    Serial.println(F("\n--- Initialisation OTA ---"));

    // Configuration du port
    ArduinoOTA.setPort(port);

    // Configuration du hostname
    ArduinoOTA.setHostname(hostname);

    // Configuration du mot de passe (si fourni)
    if (password != nullptr && strlen(password) > 0)
    {
        ArduinoOTA.setPassword(password);
        Serial.println(F("[OTA] Mot de passe activé"));
    }
    else
    {
        Serial.println(F("[OTA] ⚠️ Pas de mot de passe (non sécurisé)"));
    }

    // Configuration des callbacks
    setupCallbacks();

    // Démarrage du service
    ArduinoOTA.begin();

    initialized = true;
    enabled = true;
    currentState = OTAState::OTA_READY;

    Serial.println(F("[OTA] ✓ Service OTA initialisé"));
    Serial.print(F("[OTA] Hostname: "));
    Serial.println(hostname);
    Serial.print(F("[OTA] Port: "));
    Serial.println(port);
    Serial.print(F("[OTA] URL mDNS: "));
    Serial.println(getMDNSUrl());
    Serial.println(F("-----------------------------"));

    return true;
}

void OTAManager::end()
{
#ifdef ESP8266
    // ESP8266 n'a pas de méthode end()
    enabled = false;
#else
    ArduinoOTA.end();
    enabled = false;
#endif

    initialized = false;
    currentState = OTAState::OTA_IDLE;
    Serial.println(F("[OTA] Service arrêté"));
}

// Contrôle
void OTAManager::enable()
{
    if (initialized)
    {
        enabled = true;
        currentState = OTAState::OTA_READY;
    }
}

void OTAManager::disable()
{
    enabled = false;
    currentState = OTAState::OTA_IDLE;
}

bool OTAManager::isEnabled() const
{
    return enabled;
}

void OTAManager::handle()
{
    if (enabled && initialized)
    {
        ArduinoOTA.handle();
    }
}

// Configuration
void OTAManager::setHostname(const char *name)
{
    hostname = name;
    if (initialized)
    {
        ArduinoOTA.setHostname(hostname);
    }
}

void OTAManager::setPassword(const char *pwd)
{
    password = pwd;
    if (initialized && password != nullptr)
    {
        ArduinoOTA.setPassword(password);
    }
}

void OTAManager::setPort(uint16_t portNumber)
{
    port = portNumber;
    if (initialized)
    {
        ArduinoOTA.setPort(port);
    }
}

// Callbacks internes
void OTAManager::setupCallbacks()
{
    // Callback de démarrage
    ArduinoOTA.onStart([this]()
                       {
    currentState = OTAState::OTA_UPDATING;
    
    // Déterminer le type de mise à jour
    if (ArduinoOTA.getCommand() == U_FLASH) {
      updateType = OTA_UPDATE_SKETCH;
      Serial.println(F("\n[OTA] 🔄 Démarrage : Mise à jour du programme"));
    } else {
      updateType = OTA_UPDATE_FILESYSTEM;
      Serial.println(F("\n[OTA] 🔄 Démarrage : Mise à jour du filesystem"));
    }
    
    // Callback personnalisé
    if (onStartCallback != nullptr) {
      onStartCallback();
    } });

    // Callback de progression
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total)
                          {
    lastProgress = progress;
    lastTotal = total;
    
    unsigned int percent = (progress / (total / 100));
    Serial.printf("[OTA] Progression : %u%%\r", percent);
    
    // Callback personnalisé
    if (onProgressCallback != nullptr) {
      onProgressCallback(progress, total);
    } });

    // Callback de fin
    ArduinoOTA.onEnd([this]()
                     {
    currentState = OTAState::OTA_SUCCESS;
    Serial.println(F("\n[OTA] ✓ Mise à jour terminée !"));
    Serial.println(F("[OTA] Redémarrage en cours..."));
    
    // Callback personnalisé
    if (onEndCallback != nullptr) {
      onEndCallback();
    } });

    // Callback d'erreur
    ArduinoOTA.onError([this](ota_error_t error)
                       {
    currentState = OTAState::OTA_ERROR;
    lastError = error;
    
    Serial.printf("\n[OTA] ✗ Erreur [%u]: ", error);
    Serial.println(getErrorString(error));
    
    // Callback personnalisé
    if (onErrorCallback != nullptr) {
      onErrorCallback(error);
    } });
}

const char *OTAManager::getErrorString(int error) const
{
    switch (error)
    {
    case OTA_AUTH_ERROR:
        return "Authentification échouée (Mot de passe incorrect)";
    case OTA_BEGIN_ERROR:
        return "Erreur de démarrage";
    case OTA_CONNECT_ERROR:
        return "Erreur de connexion";
    case OTA_RECEIVE_ERROR:
        return "Erreur de réception";
    case OTA_END_ERROR:
        return "Erreur de fin";
    default:
        return "Erreur inconnue";
    }
}

// Callbacks personnalisés
void OTAManager::onStart(OTACallback callback)
{
    onStartCallback = callback;
}

void OTAManager::onEnd(OTACallback callback)
{
    onEndCallback = callback;
}

void OTAManager::onProgress(OTAProgressCallback callback)
{
    onProgressCallback = callback;
}

void OTAManager::onError(OTAErrorCallback callback)
{
    onErrorCallback = callback;
}

// Informations
OTAState OTAManager::getState() const
{
    return currentState;
}

String OTAManager::getStateString() const
{
    switch (currentState)
    {
    case OTAState::OTA_IDLE:
        return "Inactif";
    case OTAState::OTA_READY:
        return "Prêt";
    case OTAState::OTA_UPDATING:
        return "Mise à jour...";
    case OTAState::OTA_SUCCESS:
        return "Succès";
    case OTAState::OTA_ERROR:
        return "Erreur";
    default:
        return "Inconnu";
    }
}

const char *OTAManager::getHostname() const
{
    return hostname;
}

uint16_t OTAManager::getPort() const
{
    return port;
}

bool OTAManager::isUpdating() const
{
    return currentState == OTAState::OTA_UPDATING;
}

// Progression
unsigned int OTAManager::getProgress() const
{
    return lastProgress;
}

unsigned int OTAManager::getProgressPercent() const
{
    if (lastTotal == 0)
        return 0;
    return (lastProgress * 100) / lastTotal;
}

// Erreurs
int OTAManager::getLastError() const
{
    return lastError;
}

String OTAManager::getLastErrorString() const
{
    return String(getErrorString(lastError));
}

// Utilitaires
void OTAManager::printInfo()
{
    Serial.println(F("\n===== OTA Manager Info ====="));
    Serial.print(F("État: "));
    Serial.println(getStateString());
    Serial.print(F("Hostname: "));
    Serial.println(hostname);
    Serial.print(F("Port: "));
    Serial.println(port);
    Serial.print(F("URL mDNS: "));
    Serial.println(getMDNSUrl());
    Serial.print(F("Mot de passe: "));
    Serial.println((password != nullptr && strlen(password) > 0) ? "Oui" : "Non");
    Serial.print(F("Activé: "));
    Serial.println(enabled ? "Oui" : "Non");

    if (currentState == OTAState::OTA_UPDATING)
    {
        Serial.print(F("Progression: "));
        Serial.print(getProgressPercent());
        Serial.println(F("%"));
    }

    if (currentState == OTAState::OTA_ERROR)
    {
        Serial.print(F("Dernière erreur: "));
        Serial.println(getLastErrorString());
    }

    Serial.println(F("============================\n"));
}

String OTAManager::getMDNSUrl() const
{
    String url = hostname;
    url += ".local";
    if (port != 80)
    {
        url += ":";
        url += port;
    }
    return url;
}