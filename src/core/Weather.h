#ifndef WEATHER_H
#define WEATHER_H

#include <QObject>
#include <QTimer>

enum class WeatherType { Sunny, Rainy, Drought, Storm, Windy };

inline QString weatherName(WeatherType w) {
    switch (w) {
    case WeatherType::Sunny: return QStringLiteral("晴天");
    case WeatherType::Rainy: return QStringLiteral("雨天");
    case WeatherType::Drought: return QStringLiteral("干旱");
    case WeatherType::Storm: return QStringLiteral("暴雨");
    case WeatherType::Windy: return QStringLiteral("大风");
    default: return QStringLiteral("未知");
    }
}

class Weather : public QObject {
    Q_OBJECT
public:
    explicit Weather(QObject *parent = nullptr);

    WeatherType current() const { return m_current; }
    void setWeather(WeatherType w);

    // Effect multipliers
    double waterDrainMultiplier() const;
    double growthMultiplier() const;
    double bugChanceBonus() const;
    double droneSpeedMultiplier() const;

signals:
    void weatherChanged(WeatherType newWeather);

private:
    void tick();
    WeatherType m_current = WeatherType::Sunny;
    QTimer m_timer;
};

#endif // WEATHER_H
