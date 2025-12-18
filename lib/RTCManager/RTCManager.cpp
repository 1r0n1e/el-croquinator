/*
 * RTCManager.cpp
 * Implémentation de la librairie RTCManager
 */

#include "RTCManager.h"

// Noms des jours et mois
const char *const DAYS_LONG[] = {"", "Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
const char *const DAYS_SHORT[] = {"", "Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
const char *const MONTHS_LONG[] = {"", "Janvier", "Février", "Mars", "Avril", "Mai", "Juin",
                                   "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"};
const char *const MONTHS_SHORT[] = {"", "Jan", "Fév", "Mar", "Avr", "Mai", "Juin",
                                    "Juil", "Août", "Sep", "Oct", "Nov", "Déc"};

// Constructeur
RTCManager::RTCManager(uint8_t clk, uint8_t dat, uint8_t rst)
{
    clkPin = clk;
    datPin = dat;
    rstPin = rst;

    _rtc = new virtuabotixRTC(clkPin, datPin, rstPin);

    initialized = false;
    debugMode = false;

    onMidnight = nullptr;
    midnightTriggered = false;

    // Initialiser les alarmes
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        alarms[i].enabled = false;
        alarms[i].triggered = false;
        alarms[i].callback = nullptr;
    }
}

RTCManager::~RTCManager()
{
    delete _rtc;
}

// Initialisation
bool RTCManager::begin()
{
    initialized = true;

    if (debugMode)
    {
        Serial.println(F("[RTC] Initialisation..."));
    }

    // Lire l'heure actuelle pour vérifier la communication
    _rtc->updateTime();

    if (debugMode)
    {
        Serial.println(F("[RTC] ✓ Module DS1302 prêt"));
        printDateTime();
    }

    return true;
}

void RTCManager::setDebugMode(bool enable)
{
    debugMode = enable;
}

// Configuration de l'heure
void RTCManager::setDateTime(uint8_t second, uint8_t minute, uint8_t hour,
                             uint8_t dayOfWeek, uint8_t dayOfMonth,
                             uint8_t month, uint16_t year)
{
    _rtc->setDS1302Time(second, minute, hour, dayOfWeek, dayOfMonth, month, year);

    if (debugMode)
    {
        Serial.println(F("[RTC] Date/Heure configurée"));
        update();
        printDateTime();
    }
}

void RTCManager::setDateTime(const DateTime &dt)
{
    setDateTime(dt.second, dt.minute, dt.hour, dt.dayOfWeek,
                dt.dayOfMonth, dt.month, dt.year);
}

void RTCManager::setTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    _rtc->updateTime();
    setDateTime(second, minute, hour, _rtc->dayofweek,
                _rtc->dayofmonth, _rtc->month, _rtc->year);
}

void RTCManager::setDate(uint8_t day, uint8_t month, uint16_t year)
{
    _rtc->updateTime();

    // Calculer le jour de la semaine (algorithme de Zeller)
    int q = day;
    int m = month;
    int y = year;

    if (m < 3)
    {
        m += 12;
        y--;
    }

    int h = (q + (13 * (m + 1)) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
    uint8_t dayOfWeek = ((h + 6) % 7) + 1; // Conversion pour DS1302 (1=Dim)

    setDateTime(_rtc->seconds, _rtc->minutes, _rtc->hours, dayOfWeek,
                day, month, year);
}

// Synchronisation avec NTP (ESP8266 uniquement)

#ifdef ESP8266 || ESP32
bool RTCManager::syncFromNTP(long gmtOffset, int dstOffset)
{
    Serial.println(F("[RTC] Synchronisation NTP..."));
    configTime(gmtOffset, dstOffset, "pool.ntp.org");

    // Attendre la synchronisation
    int attempts = 0;
    while (time(nullptr) < 100000 && attempts < 10)
    {
        delay(500);
        attempts++;
    }

    if (time(nullptr) < 100000)
    {
        if (debugMode)
        {
            Serial.println(F("[RTC] ✗ Échec synchronisation NTP"));
        }
        return false;
    }

    // Récupérer l'heure NTP
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    // Configurer le RTC
    setDateTime(timeinfo->tm_sec, timeinfo->tm_min, timeinfo->tm_hour,
                timeinfo->tm_wday + 1, timeinfo->tm_mday,
                timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

    if (debugMode)
    {
        Serial.println(F("[RTC] ✗ Échec d'update, vérifier la batterie ou le branchement. "));
        printDateTime();
    }
    update();
    return true;
}
#endif

// Lecture de l'heure
void RTCManager::update()
{
    _rtc->updateTime();

    if (_rtc->year < 2025)
    {
        if (debugMode)
        {
            Serial.println(F("[RTC] ✓ Synchronisé avec NTP"));
            printDateTime();
        }
    }
    else
    {
        // Vérifier minuit
        checkMidnight();

        // Vérifier les alarmes
        checkAlarms();
    }
}

DateTime RTCManager::getDateTime()
{
    update();

    DateTime dt;
    dt.second = _rtc->seconds;
    dt.minute = _rtc->minutes;
    dt.hour = _rtc->hours;
    dt.dayOfWeek = _rtc->dayofweek;
    dt.dayOfMonth = _rtc->dayofmonth;
    dt.month = _rtc->month;
    dt.year = _rtc->year;

    return dt;
}

String RTCManager::getTimeString(const char *format)
{
    update();
    char buffer[20];
    snprintf(buffer, sizeof(buffer), format, _rtc->hours, _rtc->minutes, _rtc->seconds);
    return String(buffer);
}

String RTCManager::getDateString(const char *format)
{
    update();
    char buffer[20];
    snprintf(buffer, sizeof(buffer), format, _rtc->dayofmonth, _rtc->month, _rtc->year);
    return String(buffer);
}

String RTCManager::getDateTimeString()
{
    update();
    char buffer[30];
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d:%02d",
             _rtc->dayofmonth, _rtc->month, _rtc->year,
             _rtc->hours, _rtc->minutes, _rtc->seconds);
    return String(buffer);
}

// Accès aux composants individuels
uint8_t RTCManager::getSecond()
{
    update();
    return _rtc->seconds;
}
uint8_t RTCManager::getMinute()
{
    update();
    return _rtc->minutes;
}
uint8_t RTCManager::getHour()
{
    update();
    return _rtc->hours;
}
uint8_t RTCManager::getDayOfWeek()
{
    update();
    return _rtc->dayofweek;
}
uint8_t RTCManager::getDayOfMonth()
{
    update();
    return _rtc->dayofmonth;
}
uint8_t RTCManager::getMonth()
{
    update();
    return _rtc->month;
}
uint16_t RTCManager::getYear()
{
    update();
    return _rtc->year;
}

// Conversions temporelles
unsigned long RTCManager::getSecondsFromMidnight()
{
    update();
    return (unsigned long)_rtc->hours * 3600UL +
           (unsigned long)_rtc->minutes * 60UL +
           (unsigned long)_rtc->seconds;
}

unsigned long RTCManager::getSecondsFromEpoch()
{
    update();

    struct tm timeinfo;
    timeinfo.tm_sec = _rtc->seconds;
    timeinfo.tm_min = _rtc->minutes;
    timeinfo.tm_hour = _rtc->hours;
    timeinfo.tm_mday = _rtc->dayofmonth;
    timeinfo.tm_mon = _rtc->month - 1;
    timeinfo.tm_year = _rtc->year - 1900;

    return mktime(&timeinfo);
}

void RTCManager::setFromSecondsFromEpoch(unsigned long seconds)
{
    time_t rawtime = seconds;
    struct tm *timeinfo = localtime(&rawtime);

    setDateTime(timeinfo->tm_sec, timeinfo->tm_min, timeinfo->tm_hour,
                timeinfo->tm_wday + 1, timeinfo->tm_mday,
                timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
}

// Formatage du temps
String RTCManager::formatSecondsToTime(unsigned long seconds, bool showSeconds)
{
    seconds = seconds % 86400; // Limiter à 24h

    uint8_t h = seconds / 3600;
    uint8_t m = (seconds % 3600) / 60;
    uint8_t s = seconds % 60;

    char buffer[12];
    if (showSeconds)
    {
        snprintf(buffer, sizeof(buffer), "%02dh%02dm%02ds", h, m, s);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%02dh%02d", h, m);
    }

    return String(buffer);
}

String RTCManager::formatDuration(unsigned long seconds)
{
    if (seconds < 0)
        return "0s";

    uint8_t h = seconds / 3600;
    uint8_t m = (seconds % 3600) / 60;
    uint8_t s = seconds % 60;

    String result = "";
    if (h > 0)
        result += String(h) + "h ";
    if (m > 0 || h > 0)
        result += String(m) + "m ";
    result += String(s) + "s";

    return result;
}

// Vérifications de plages horaires
bool RTCManager::isInTimeRange(uint8_t startH, uint8_t startM, uint8_t endH, uint8_t endM)
{
    update();

    unsigned long current = _rtc->hours * 60UL + _rtc->minutes;
    unsigned long start = startH * 60UL + startM;
    unsigned long end = endH * 60UL + endM;

    // Gérer le cas où la plage traverse minuit
    if (end < start)
    {
        return (current >= start || current <= end);
    }

    return (current >= start && current <= end);
}

bool RTCManager::isInTimeRange(const TimeRange &range)
{
    return isInTimeRange(range.startHour, range.startMinute,
                         range.endHour, range.endMinute);
}

// Jours de la semaine
String RTCManager::getDayName(bool shortName)
{
    update();
    if (_rtc->dayofweek < 1 || _rtc->dayofweek > 7)
        return "?";
    return String(shortName ? DAYS_SHORT[_rtc->dayofweek] : DAYS_LONG[_rtc->dayofweek]);
}

String RTCManager::getMonthName(bool shortName)
{
    update();
    if (_rtc->month < 1 || _rtc->month > 12)
        return "?";
    return String(shortName ? MONTHS_SHORT[_rtc->month] : MONTHS_LONG[_rtc->month]);
}

bool RTCManager::isWeekend()
{
    update();
    return (_rtc->dayofweek == SUNDAY || _rtc->dayofweek == SATURDAY);
}

bool RTCManager::isWeekday()
{
    return !isWeekend();
}

// Alarmes
int RTCManager::addAlarm(uint8_t hour, uint8_t minute, AlarmCallback callback)
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        if (!alarms[i].enabled)
        {
            alarms[i].enabled = true;
            alarms[i].hour = hour;
            alarms[i].minute = minute;
            alarms[i].triggered = false;
            alarms[i].callback = callback;

            if (debugMode)
            {
                Serial.print(F("[RTC] Alarme ajoutée #"));
                Serial.print(i);
                Serial.print(F(" à "));
                Serial.print(hour);
                Serial.print(":");
                Serial.println(minute);
            }

            return i;
        }
    }

    return -1; // Pas d'emplacement disponible
}

void RTCManager::removeAlarm(int alarmId)
{
    if (alarmId >= 0 && alarmId < MAX_ALARMS)
    {
        alarms[alarmId].enabled = false;
        alarms[alarmId].callback = nullptr;
    }
}

void RTCManager::enableAlarm(int alarmId)
{
    if (alarmId >= 0 && alarmId < MAX_ALARMS)
    {
        alarms[alarmId].enabled = true;
        alarms[alarmId].triggered = false;
    }
}

void RTCManager::disableAlarm(int alarmId)
{
    if (alarmId >= 0 && alarmId < MAX_ALARMS)
    {
        alarms[alarmId].enabled = false;
    }
}

void RTCManager::clearAllAlarms()
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        alarms[i].enabled = false;
        alarms[i].callback = nullptr;
    }
}

// Callback à minuit
void RTCManager::setMidnightCallback(MidnightCallback callback)
{
    onMidnight = callback;
}

// Méthodes internes
void RTCManager::checkMidnight()
{
    if (_rtc->hours == 0 && _rtc->minutes == 0 && _rtc->seconds == 0)
    {
        if (!midnightTriggered)
        {
            midnightTriggered = true;

            if (debugMode)
            {
                Serial.println(F("[RTC] ⏰ Minuit !"));
            }

            if (onMidnight != nullptr)
            {
                onMidnight();
            }
        }
    }
    else
    {
        midnightTriggered = false;
    }
}

void RTCManager::checkAlarms()
{
    for (int i = 0; i < MAX_ALARMS; i++)
    {
        if (alarms[i].enabled)
        {
            if (_rtc->hours == alarms[i].hour &&
                _rtc->minutes == alarms[i].minute &&
                _rtc->seconds == 0)
            {

                if (!alarms[i].triggered)
                {
                    alarms[i].triggered = true;

                    if (debugMode)
                    {
                        Serial.print(F("[RTC] 🔔 Alarme #"));
                        Serial.println(i);
                    }

                    if (alarms[i].callback != nullptr)
                    {
                        alarms[i].callback();
                    }
                }
            }
            else
            {
                alarms[i].triggered = false;
            }
        }
    }
}

// Calculs de temps
long RTCManager::getTimeDifferenceSeconds(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2)
{
    long time1 = h1 * 3600L + m1 * 60L;
    long time2 = h2 * 3600L + m2 * 60L;
    return time2 - time1;
}

bool RTCManager::isBefore(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2)
{
    return getTimeDifferenceSeconds(h1, m1, h2, m2) > 0;
}

bool RTCManager::isAfter(uint8_t h1, uint8_t m1, uint8_t h2, uint8_t m2)
{
    return getTimeDifferenceSeconds(h1, m1, h2, m2) < 0;
}

// Comparaisons de dates
bool RTCManager::isSameDay(const DateTime &dt1, const DateTime &dt2)
{
    return (dt1.dayOfMonth == dt2.dayOfMonth &&
            dt1.month == dt2.month &&
            dt1.year == dt2.year);
}

bool RTCManager::isSameMonth(const DateTime &dt1, const DateTime &dt2)
{
    return (dt1.month == dt2.month && dt1.year == dt2.year);
}

bool RTCManager::isLeapYear(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

uint8_t RTCManager::getDaysInMonth(uint8_t month, uint16_t year)
{
    const uint8_t daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year))
        return 29;
    return daysInMonth[month];
}

int RTCManager::daysBetween(const DateTime &dt1, const DateTime &dt2)
{
    // Conversion simplifiée (approximation)
    long days1 = dt1.year * 365L + dt1.month * 30L + dt1.dayOfMonth;
    long days2 = dt2.year * 365L + dt2.month * 30L + dt2.dayOfMonth;
    return days2 - days1;
}

// Utilitaires
void RTCManager::printInfo()
{
    Serial.println(F("\n===== RTC Manager Info ====="));
    Serial.print(F("Module: DS1302 (CLK="));
    Serial.print(clkPin);
    Serial.print(F(", DAT="));
    Serial.print(datPin);
    Serial.print(F(", RST="));
    Serial.print(rstPin);
    Serial.println(F(")"));
    printDateTime();
    Serial.println(F("============================\n"));
}

void RTCManager::printDateTime()
{
    update();

    Serial.print(F("[RTC] "));
    Serial.print(getDayName());
    Serial.print(F(" "));
    Serial.print(_rtc->dayofmonth);
    Serial.print(F(" "));
    Serial.print(getMonthName());
    Serial.print(F(" "));
    Serial.print(_rtc->year);
    Serial.print(F(" - "));
    Serial.print(_rtc->hours);
    Serial.print(F(":"));
    if (_rtc->minutes < 10)
        Serial.print("0");
    Serial.print(_rtc->minutes);
    Serial.print(F(":"));
    if (_rtc->seconds < 10)
        Serial.print("0");
    Serial.println(_rtc->seconds);
}

virtuabotixRTC *RTCManager::getRTC()
{
    return _rtc;
}