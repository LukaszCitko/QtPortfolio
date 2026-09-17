#include "pump.h"



Pump::Pump(QObject *parent)
    : QObject{parent}
    , m_state(State::Stopped)
    , m_targetRpm(1500.0)
    , m_actualRpm(0.0)
    , m_temperatureFromSensor(25.0)
{

}


double Pump::temperatureFromSensor() const
{
    return m_temperatureFromSensor;
}

void Pump::setTemperatureFromSensor(double newTemperature)
{
    if (qFuzzyCompare(m_temperatureFromSensor, newTemperature))
    {
        return;
    }

    m_temperatureFromSensor = newTemperature;
    emit temperatureFromSensorChanged();
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

void Pump::setTargetRpm(double rpm)
{
    if (qFuzzyCompare(m_targetRpm, rpm))  // maybe not necessary if we will jumps every 10%
    {
        return;
    }

    m_targetRpm = rpm;
    emit targetRpmChanged();
}


double Pump::targetRpm() const
{
    return m_targetRpm;
}

void Pump::setActualRpm(double rpm)
{
    if (qFuzzyCompare(m_actualRpm, rpm))
    {
        return;
    }

    m_actualRpm = rpm;
    emit actualRpmChanged();
}

double Pump::actualRpm() const
{
    return  m_actualRpm;
}