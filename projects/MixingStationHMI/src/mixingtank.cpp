#include <algorithm>
#include <cmath>

#include "mixingtank.h"

MixingTank::MixingTank(QObject *parent)
    : QObject(parent),
    m_volume(0.0),
    m_temperature(25.0),
    m_waterVolume(0.0),
    m_concentrateVolume(0.0),
    m_state(State::Empty)
{
}

double MixingTank::capacity() const
{
    return Capacity;
}

double MixingTank::volume() const
{
    return m_volume;
}

double MixingTank::levelPercent() const
{
    return (m_volume / 100.0) * 100.0;
}

double MixingTank::temperature() const
{
    return m_temperature;
}

double MixingTank::waterVolume() const
{
    return m_waterVolume;
}

double MixingTank::concentrateVolume() const
{
    return m_concentrateVolume;
}

MixingTank::State MixingTank::state() const
{
    return m_state;
}

bool MixingTank::hasFault() const
{
    return m_state == State::Fault;
}

bool MixingTank::isActive() const
{
    switch (m_state)
    {
    case State::Filling:
    case State::Mixing:
    case State::Transferring:
    case State::Draining:
        return true;

    default:
        return false;
    }
}

QString MixingTank::stateText() const
{
    switch (m_state)
    {
    case State::Empty:
        return "EMPTY";

    case State::Filling:
        return "FILLING";

    case State::ReadyForMixing:
        return "READY FOR MIXING";

    case State::Mixing:
        return "MIXING";

    case State::ReadyForTransfer:
        return "READY FOR TRANSFER";

    case State::Transferring:
        return "TRANSFERRING";

    case State::Draining:
        return "DRAINING";

    case State::Complete:
        return "COMPLETE";

    case State::Fault:
        return "FAULT";
    }

    return "UNKNOWN";
}

void MixingTank::addWater(double amount)
{
    if (amount <= 0.0)
        return;

    const double availableCapacity = Capacity - m_volume;
    const double addedVolume = (amount < availableCapacity)
                                   ? amount
                                   : availableCapacity;

    if (addedVolume <= 0.0)
        return;

    m_volume += addedVolume;
    m_waterVolume += addedVolume;

    emit volumeChanged();
    emit compositionChanged();

    if (m_state == State::Empty)
        setState(State::Filling);
}

void MixingTank::addConcentrate(double amount)
{
    if (amount <= 0.0)
        return;

    const double availableCapacity = Capacity - m_volume;
    const double addedVolume = (amount < availableCapacity)
                                   ? amount
                                   : availableCapacity;

    if (addedVolume <= 0.0)
        return;

    m_volume += addedVolume;
    m_concentrateVolume += addedVolume;

    emit volumeChanged();
    emit compositionChanged();

    if (m_state == State::Empty)
        setState(State::Filling);
}

void MixingTank::setTemperature(double newTemperature)
{
    if (m_temperature == newTemperature)
        return;

    m_temperature = newTemperature;
    emit temperatureChanged();
}

void MixingTank::setState(State newState)
{
    if (m_state == newState)
        return;

    m_state = newState;
    emit stateChanged();
}

void MixingTank::removeProduct(double amount)
{
    if (amount <= 0.0)
        return;

    if (m_volume <= 0.0)
        return;

    const double removedVolume =
        (amount < m_volume)
            ? amount
            : m_volume;

    m_volume -= removedVolume;

    if (m_volume < 0.0)
        m_volume = 0.0;

    emit volumeChanged();
}

bool MixingTank::resetAfterTransfer()
{
    if (m_state != State::Complete || m_volume != 0.0)
        return false;

    m_waterVolume = 0.0;
    m_concentrateVolume = 0.0;
    m_temperature = 25.0;

    emit compositionChanged();
    emit temperatureChanged();
    setState(State::Empty);

    return true;
}

bool MixingTank::resetAfterDrain()
{
    if (m_state != State::Draining || m_volume != 0.0)
        return false;

    m_waterVolume = 0.0;
    m_concentrateVolume = 0.0;
    m_temperature = 25.0;

    emit compositionChanged();
    emit temperatureChanged();
    setState(State::Empty);

    return true;
}

void MixingTank::synchronizeFromController(double waterVolume, double concentrateVolume, double temperature, State state)
{
    if (!std::isfinite(waterVolume) || !std::isfinite(concentrateVolume) || !std::isfinite(temperature))
    {
        return;
    }

    const double boundedWaterVolume = std::clamp(waterVolume, 0.0, Capacity);
    const double boundedConcentrateVolume = std::clamp(concentrateVolume, 0.0, Capacity - boundedWaterVolume);
    const double totalVolume = boundedWaterVolume + boundedConcentrateVolume;
    const bool volumeHasChanged = !qFuzzyCompare(m_volume + 1.0, totalVolume + 1.0);
    const bool compositionHasChanged = !qFuzzyCompare(m_waterVolume + 1.0, boundedWaterVolume + 1.0) || !qFuzzyCompare(m_concentrateVolume + 1.0, boundedConcentrateVolume + 1.0);
    const bool temperatureHasChanged = !qFuzzyCompare(m_temperature + 1.0, temperature + 1.0);
    const bool stateHasChanged = m_state != state;

    m_volume = totalVolume;
    m_waterVolume = boundedWaterVolume;
    m_concentrateVolume = boundedConcentrateVolume;
    m_temperature = temperature;
    m_state = state;

    if (volumeHasChanged)
    {
        emit volumeChanged();
    }

    if (compositionHasChanged)
    {
        emit compositionChanged();
    }

    if (temperatureHasChanged)
    {
        emit temperatureChanged();
    }

    if (stateHasChanged)
    {
        emit stateChanged();
    }
}
