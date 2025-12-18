/*
 * RTCManager.h
 * Librairie complète pour gérer le module RTC DS1302
 * Basée sur virtuabotixRTC avec fonctionnalités étendues
 */

#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <virtuabotixRTC.h>
#include <time.h>

// Structure pour une plage horaire
struct TimeRange
{
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t endHour;
    uint8_t endMinute;
};

// Structure pour une date/heure complète
struct DateTime
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t dayOfWeek; // 1=Dimanche, 2=Lundi, ..., 7=Samedi
    uint8_t dayOfMonth;
    uint8_t month;
    uint16_t year;
};

// Jours de la semaine
enum DayOfWeek
{
    SUNDAY = 1,
    MONDAY = 2,
    TUESDAY = 3,
    WEDNESDAY = 4,
    THURSDAY = 5,
    FRIDAY = 6,
    SATURDAY = 7
};

// Type de callback
typedef void (*MidnightCallback)();
typedef void (*AlarmCallback)();

class RTCManager
{
private:
    virtuabotixRTC *_rtc;

    // Broches
    uint8_t clkPin;
    uint8_t datPin;
    uint8_t rstPin;

    // Configuration
    bool initialized;
    bool debugMode;

    // Alarme/Callback à minuit
    MidnightCallback onMidnight;
    bool midnightTriggered;

    // Alarmes multiples
    struct Alarm
    {
        bool enabled;
        uint8_t hour;
        uint8_t minute;
        bool triggered;
        AlarmCallback callback;
    };
    static const int MAX_ALARMS = 5;
    Alarm alarms[MAX_ALARMS];

    // Méthodes internes
    void checkMidnight();
    void checkAlarms();
    bool isLeapYear(uint16_t year);
    uint8_t getDaysInMonth(uint8_t month, uint16_t year);

public:
    // Constructeur
    RTCManager(uint8_t clk, uint8_t dat, uint8_t rst);
    ~RTCManager();

    // Initialisation
    bool begin();
    void setDebugMode(bool enable);

    // Configuration de l'heure
    void setDateTime(uint8_t second, uint8_t minute, uint8_t hour,
                     uint8_t dayOfWeek, uint8_t dayOfMonth,
                     uint8_t month, uint16_t year);
    void setDateTime(const DateTime &dt);
    void setTime(uint8_t hour, uint8_t minute, uint8_t second);
    void setDate(uint8_t day, uint8_t month, uint16_t year);

// Synchronisation avec NTP (nécessite WiFi)
#ifdef ESP8266
    bool syncFromNTP(long gmtOffset = 0, int dstOffset = 0);
#endif

    // Lecture de l'heure
    void update();
    DateTime getDateTime();
    String getTimeString(const char *format = "%02d:%02d:%02d");
    String getDateString(const char *format = "%02d/%02d/%04d");
    String getDateTimeString();

    // Accès aux composants individuels
    uint8_t getSecond();
    uint8_t getMinute();
    uint8_t getHour();
    uint8_t getDayOfWeek();
    uint8_t getDayOfMonth();
    uint8_t getMonth();
    uint16_t getYear();

    // Conversions temporelles
    unsigned long getSecondsFromMidnight();
    unsigned long getSecondsFromEpoch(); // Unix timestamp
    void setFromSecondsFromEpoch(unsigned long seconds);

    // Formatage du temps
    String formatSecondsToTime(unsigned long seconds, bool showSeconds = false);
    String formatDuration(unsigned long seconds); // Ex: "2h 15m 30s"

    // Vérifications de plages horaires
    bool isInTimeRange(uint8_t startH, uint8_t startM, uint8_t endH, uint8_t endM);
    bool isInTimeRange(const TimeRange &range);

    // Jours de la semaine
    String getDayName(bool shortName = false);
    String getMonthName(bool shortName = false);
    bool isWeekend();
    bool isWeekday();

    // Alarmes
    int addAlarm(uint8_t hour, uint8_t minute, AlarmCallback callback);
    void removeAlarm(int alarmId);
    void enableAlarm(int alarmId);
    void disableAlarm(int alarmId);
    void clearAllAlarms();

    // Callback à minuit
    void setMidnightCallback(MidnightCallback callback);

    // Calculs de temps
    long getTimeDifferenceSeconds(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2);
    bool isBefore(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2);
    bool isAfter(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2);

    // Comparaisons de dates
    bool isSameDay(const DateTime &dt1, const DateTime &dt2);
    bool isSameMonth(const DateTime &dt1, const DateTime &dt2);
    int daysBetween(const DateTime &dt1, const DateTime &dt2);

    // Utilitaires
    void printInfo();
    void printDateTime();

    // Accès direct à l'objet RTC
    virtuabotixRTC *getRTC();
};

#endif // RTC_MANAGER_H