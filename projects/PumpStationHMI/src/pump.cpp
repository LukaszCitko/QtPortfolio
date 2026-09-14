#include "pump.h"

Pump::Pump(QObject *parent)
    : QObject{parent}, m_state(State::Stopped)
{

}

void Pump::start()
{
    if (m_state != State::Fault)
    {
        m_state = State::Running;
    }
}

void Pump::stop()
{
    if (m_state != State::Fault)
    {
        m_state = State::Stopped;

    }
}

void Pump::setFault()
{
    m_state = State::Fault;

}
void Pump::resetFault()
{
    m_state = State::Stopped;
}

bool Pump::isRunning() const
{
    return m_state == State::Running;
}

Pump::State Pump::state() const
{
    return m_state;
}