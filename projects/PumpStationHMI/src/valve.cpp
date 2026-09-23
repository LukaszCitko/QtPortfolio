#include "valve.h"

Valve::Valve(int id, QObject *parent)
    : QObject{parent}
    , m_id(id)
    , m_state(State::Closed)
{
}

int Valve::id() const
{
    return m_id;
}

void Valve::open()
{
    if (m_state == State::Fault || m_state == State::Open)
    {
        return;
    }
    m_state = State::Open;
    emit stateChanged();
}

void Valve::close()
{
    if (m_state == State::Fault || m_state == State::Closed)
    {
        return;
    }

    m_state = State::Closed;
    emit stateChanged();
}

void Valve::setFault()
{
    if (m_state == State::Fault)
    {
        return;
    }

    m_state = State::Fault;
    emit stateChanged();
}

void Valve::resetFault()
{
    if (m_state != State::Fault)
    {
        return;
    }

    m_state = State::Closed;
    emit stateChanged();
}
bool Valve::isOpen() const
{
    return m_state == State::Open;
}

Valve::State Valve::state() const
{
    return m_state;
}

QString Valve::stateText() const
{
    switch (m_state)
    {
    case State::Closed:
        return "CLOSED";

    case State::Open:
        return "OPEN";

    case State::Fault:
        return "FAULT";
    }

    return "UNKNOWN";
}