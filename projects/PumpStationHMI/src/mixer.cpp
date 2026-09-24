#include "mixer.h"

Mixer::Mixer(QObject *parent)
    : QObject(parent),
    m_state(State::Stopped),
    m_connected(false)
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
    if (m_state == State::Stopped)
    {
        return;
    }

    m_state = State::Stopped;
    emit stateChanged();
}

void Mixer::setFault()
{
    if (m_state == State::Fault)
    {
        return;
    }

    m_state = State::Fault;
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