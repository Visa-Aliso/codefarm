#include "Weather.h"
#include <QRandomGenerator>

Weather::Weather(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &Weather::tick);
    m_timer.start(10000); // check weather change every 10s
}

void Weather::setWeather(WeatherType w) {
    if (m_current != w) {
        m_current = w;
        emit weatherChanged(m_current);
    }
}

void Weather::tick() {
    int r = QRandomGenerator::global()->bounded(100);
    if (r < 60) return; // 60% no change

    // Weighted random transition
    int t = QRandomGenerator::global()->bounded(100);
    if (t < 50)
        setWeather(WeatherType::Sunny);
    else if (t < 70)
        setWeather(WeatherType::Rainy);
    else if (t < 82)
        setWeather(WeatherType::Drought);
    else if (t < 92)
        setWeather(WeatherType::Storm);
    else
        setWeather(WeatherType::Windy);
}

double Weather::waterDrainMultiplier() const {
    switch (m_current) {
    case WeatherType::Drought: return 3.0;
    case WeatherType::Rainy:   return -0.5; // negative = auto-water
    default: return 1.0;
    }
}

double Weather::growthMultiplier() const {
    switch (m_current) {
    case WeatherType::Sunny: return 1.0;
    case WeatherType::Rainy: return 0.9;
    case WeatherType::Drought: return 0.6;
    case WeatherType::Storm: return 0.7;
    case WeatherType::Windy: return 0.85;
    default: return 1.0;
    }
}

double Weather::bugChanceBonus() const {
    switch (m_current) {
    case WeatherType::Storm: return 0.15;
    default: return 0.0;
    }
}

double Weather::droneSpeedMultiplier() const {
    switch (m_current) {
    case WeatherType::Windy: return 0.5;
    default: return 1.0;
    }
}
