#include "mixer.h"
#include <algorithm>
#include <cmath>

Mixer::Mixer(QObject *parent)
    : QObject(parent),
    m_state(State::Stopped),
    m_connected(false),
    m_targetRpm(0.0),
    m_actualRpm(0.0)
{
}

Mixer::State Mixer::state() const
{
    return m_state;
}

bool Mixer::isConnected() const
{
    return m_connected;
}

bool Mixer::isRunning() const
{
    return m_state == State::Running;
}

QString Mixer::stateText() const
{
    switch (m_state)
    {
    case State::Stopped:
        return "STOPPED";

    case State::Running:
        return "RUNNING";

    case State::Fault:
        return "FAULT";
    }

    return "UNKNOWN";
}

void Mixer::connect()
{
    if (m_connected)
    {
        return;
    }

    m_connected = true;
    emit connectionChanged();
}

void Mixer::start()
{
    if (!m_connected)
    {
        return;
    }

    if (m_state == State::Fault)
    {
        return;
    }

    if (m_state == State::Running)
    {
        return;
    }

    m_state = State::Running;
    emit stateChanged();
}

void Mixer::stop()
{
    if (m_state == State::Stopped || m_state == State::Fault)
        return;

    m_state = State::Stopped;
    clearRpm();
    emit stateChanged();
}

void Mixer::setFault()
{
    if (m_state == State::Fault)
        return;

    m_state = State::Fault;
    clearRpm();
    emit stateChanged();
}

void Mixer::resetFault()
{
    if (m_state != State::Fault)
    {
        return;
    }

    m_state = State::Stopped;
    emit stateChanged();
}
double Mixer::targetRpm() const
{
    return m_targetRpm;
}

double Mixer::actualRpm() const
{
    return m_actualRpm;
}

void Mixer::setTargetRpm(double rpm)
{
    if (!isRunning() || !std::isfinite(rpm))
        return;

    const double boundedRpm = std::clamp(rpm, 0.0, MaxRpm);

    if (m_targetRpm == boundedRpm)
        return;

    m_targetRpm = boundedRpm;
    emit targetRpmChanged();
}

void Mixer::simulateStep(double elapsedSeconds)
{
    if (!isRunning() || elapsedSeconds <= 0.0)
        return;

    constexpr double RpmChangePerSecond = 400.0;
    const double maxChange = RpmChangePerSecond * elapsedSeconds;
    const double difference = m_targetRpm - m_actualRpm;
    const double change = std::clamp(difference, -maxChange, maxChange);

    if (change == 0.0)
        return;

    m_actualRpm += change;
    emit actualRpmChanged();
}

void Mixer::clearRpm()
{
    if (m_targetRpm != 0.0)
    {
        m_targetRpm = 0.0;
        emit targetRpmChanged();
    }

    if (m_actualRpm != 0.0)
    {
        m_actualRpm = 0.0;
        emit actualRpmChanged();
    }
}