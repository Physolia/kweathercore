#pragma once
#include "weatherforecast.h"
#include <QObject>

class QNetworkReply;
class QNetworkAccessManager;
namespace KWeatherCore
{
class SunriseSource;
class PendingWeatherForecast;
class PendingWeatherForecastPrivate
{
public:
    PendingWeatherForecastPrivate(PendingWeatherForecast *qq);

    void parseWeatherForecastResults(QNetworkReply *ret);
    void parseSunriseResults();
    void parseTimezoneResult(const QString &timezone);

    void parseOneElement(const QJsonObject &obj, std::vector<HourlyWeatherForecast> &hourlyForecast);
    void getTimezone(double latitude, double longitude);
    void getSunrise();
    bool isDayTime(const QDateTime &dt) const;
    void applySunriseToForecast();

    WeatherForecast forecast;
    PendingWeatherForecast *q = nullptr;
    bool isFinished = false;
    bool hasTimezone = false;
    bool hasSunrise = false;
    QString m_timezone;

    std::vector<HourlyWeatherForecast> hourlyForecast; // tmp hourly vector

    SunriseSource *m_sunriseSource = nullptr;

    QDateTime m_expiresTime;
    QNetworkAccessManager *m_manager = nullptr;
};
}
