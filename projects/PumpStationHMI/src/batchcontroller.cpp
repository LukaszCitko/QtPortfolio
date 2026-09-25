#include "batchcontroller.h"

#include "mixingtank.h"
#include "pump.h"
#include "valve.h"
#include "mixer.h"

BatchController::BatchController(
    Pump *pump1, Valve *valve1,
    Pump *pump2, Valve *valve2,
    Pump *pump3, Valve *valve3,
    Valve *valve4,
    Mixer *mixer, MixingTank *mixingTank,
    QObject *parent)
    : QObject(parent),
    m_pump1(pump1), m_valve1(valve1),
    m_pump2(pump2), m_valve2(valve2),
    m_pump3(pump3), m_valve3(valve3),
    m_valve4(valve4),
    m_mixer(mixer), m_mixingTank(mixingTank),
    m_state(State::Idle),
    m_previousState(State::Idle),
    m_stopReason(StopReason::None),
    m_mixingElapsedSeconds(0.0)
{
    Q_ASSERT(m_pump1);
    Q_ASSERT(m_valve1);
    Q_ASSERT(m_pump2);
    Q_ASSERT(m_valve2);
    Q_ASSERT(m_pump3);
    Q_ASSERT(m_valve3);
    Q_ASSERT(m_valve4);
    Q_ASSERT(m_mixingTank);

    connect(m_mixingTank, &MixingTank::volumeChanged, this, &BatchController::onTankVolumeChanged);
    connect(m_mixingTank, &MixingTank::temperatureChanged, this, &BatchController::onTankTemperatureChanged);
    connect(m_mixer, &Mixer::stateChanged, this, &BatchController::onMixerStateChanged);
    connect(m_pump1, &Pump::stateChanged, this, &BatchController::onPump1StateChanged);
    connect(m_pump2, &Pump::stateChanged, this, &BatchController::onPump2StateChanged);
    connect(m_pump3, &Pump::stateChanged, this, &BatchController::onPump3StateChanged);
}

BatchController::State BatchController::state() const
{
    return m_state;
}

QString BatchController::stateText() const
{
    switch (m_state)
    {
    case State::Idle:
        return "IDLE";
    case State::FillingWater:
        return "FILLING WATER";
    case State::DosingConcentrate:
        return "DOSING CONCENTRATE";
    case State::TemperatureCheck:
        return "TEMPERATURE CHECK";
    case State::Mixing:
        return "MIXING";
    case State::ReadyForTransfer:
        return "READY FOR TRANSFER";
    case State::Transferring:
        return "TRANSFERRING";
    case State::Paused:
        return "PAUSED";
    case State::Complete:
        return "COMPLETE";
    }
    return "UNKNOWN";
}

BatchController::StopReason BatchController::stopReason() const
{
    return m_stopReason;
}

QString BatchController::stopReasonText() const
{
    switch (m_stopReason)
    {
    case StopReason::None:
        return "NONE";
    case StopReason::OperatorPause:
        return "OPERATOR PAUSE";
    case StopReason::MixerFault:
        return "MIXER FAULT";
    case StopReason::Pump1Fault:
        return "PUMP 1 FAULT";
    case StopReason::Pump2Fault:
        return "PUMP 2 FAULT";
    case StopReason::Pump3Fault:
        return "PUMP 3 FAULT";
    case StopReason::Valve1Fault:
        return "VALVE 1 FAULT";
    case StopReason::Valve2Fault:
        return "VALVE 2 FAULT";
    case StopReason::Valve3Fault:
        return "VALVE 3 FAULT";
    }
    return "UNKNOWN";
}

void BatchController::startBatch()
{
    if (m_state != State::Idle)
        return;

    startWaterFilling();
}

void BatchController::simulateStep(double elapsedSeconds)
{
    if (elapsedSeconds <= 0.0)
        return;

    if (m_state != State::Mixing)
        return;

    m_mixingElapsedSeconds += elapsedSeconds;

    if (m_mixingElapsedSeconds >= MixingDuration)
    {
        finishMixing();
    }
}

void BatchController::startWaterFilling()
{
    if (m_pump1->state() == Pump::State::Fault)
    {
        return;
    }

    m_valve1->open();

    if (!canStartPump1())
        return;

    m_state = State::FillingWater;
    emit stateChanged();


    m_pump1->start();
}

void BatchController::startConcentrateDosing()
{
    if (m_pump2->state() == Pump::State::Fault)
    {
        m_pump1->stop();
        m_valve1->close();

        m_previousState = State::DosingConcentrate;
        m_stopReason = StopReason::Pump2Fault;
        m_state = State::Paused;
        emit stateChanged();
        return;
    }

    m_pump1->stop();
    m_valve1->close();
    m_mixer->start();

    if (!m_mixer->isRunning())
    {
        return;
    }

    m_valve2->open();

    if (!canStartPump2())
        return;

    m_state = State::DosingConcentrate;
    emit stateChanged();

    m_pump2->start();
}

void BatchController::startTemperatureCheck()
{
    m_pump2->stop();
    m_valve2->close();

    m_state = State::TemperatureCheck;
    emit stateChanged();
}

void BatchController::startMixing()
{
    m_mixingElapsedSeconds = 0.0;

    m_state = State::Mixing;
    emit stateChanged();
}

void BatchController::finishMixing()
{
    m_mixer->stop();

    m_state = State::ReadyForTransfer;
    emit stateChanged();
}

void BatchController::onTankVolumeChanged()
{
    if (m_state == State::FillingWater)
    {
        if (m_mixingTank->waterVolume() >= TargetWaterVolume)
        {
            startConcentrateDosing();
        }
        return;
    }
    if (m_state == State::DosingConcentrate)
    {
        if (m_mixingTank->concentrateVolume() >= TargetConcentrateVolume)
        {
            startTemperatureCheck();
        }
        return;
    }
    if (m_state == State::Transferring)
    {
        if (m_mixingTank->volume() <= 0.0)
        {
            finishTransfer();
        }
    }
}

void BatchController::onTankTemperatureChanged()
{
    if (m_state != State::TemperatureCheck)
        return;

    const double temperature = m_mixingTank->temperature();

    if (temperature < MinMixingTemperature ||
        temperature > MaxMixingTemperature)
    {
        return;
    }

    startMixing();
}

void BatchController::startTransfer()
{
    if (m_state != State::ReadyForTransfer)
        return;

    if (m_pump3->state() == Pump::State::Fault)
        return;

    m_valve3->open();

    if (!canStartPump3())
        return;

    m_state = State::Transferring;
    emit stateChanged();

    m_pump3->start();
}

void BatchController::finishTransfer()
{
    m_pump3->stop();
    m_valve3->close();

    m_state = State::Complete;
    emit stateChanged();
}


bool BatchController::canStartPump1() const
{
    return m_valve1->isOpen();
}

bool BatchController::canStartPump2() const
{
    return m_valve2->isOpen();
}

bool BatchController::canStartPump3() const
{
    return m_valve3->isOpen() &&
           m_state == State::ReadyForTransfer;
}

void BatchController::emergencyDrain()
{
    m_pump1->stop();
    m_pump2->stop();
    m_pump3->stop();
    m_mixer->stop();

    m_valve1->close();
    m_valve2->close();
    m_valve3->close();

    m_valve4->open();
    m_state = State::Idle;
    emit stateChanged();
}

void BatchController::pause()
{
    if (m_state == State::Idle ||
        m_state == State::ReadyForTransfer ||
        m_state == State::Complete ||
        m_state == State::Paused)
    {
        return;
    }

    m_previousState = m_state;
    m_stopReason = StopReason::OperatorPause;


    if (m_state == State::FillingWater)
    {
        m_pump1->stop();
        m_valve1->close();
    }

    else if (m_state == State::DosingConcentrate)
    {
        m_pump2->stop();
        m_valve2->close();
        // Mixer remains running during pause.
    }

    else if (m_state == State::Mixing)
    {
        m_mixer->stop();
    }


    else if (m_state == State::Transferring)
    {
        m_pump3->stop();
        m_valve3->close();
    }

    m_state = State::Paused;
    emit stateChanged();

}


void BatchController::resume()
{
    if (m_state != State::Paused)
    {
        return;
    }

    if (m_previousState == State::FillingWater)
    {
        if (m_pump1->state() == Pump::State::Fault)
        {
            return;
        }

        m_valve1->open();

        if (!canStartPump1())
        {
            return;
        }

        m_state = State::FillingWater;
        emit stateChanged();

        m_pump1->start();
    }

    else if (m_previousState == State::DosingConcentrate)
    {
        if (m_pump2->state() == Pump::State::Fault)
        {
            return;
        }

        m_mixer->start();

        if (!m_mixer->isRunning())
        {
            return;
        }

        m_valve2->open();

        if (!canStartPump2())
        {
            return;
        }

        m_state = State::DosingConcentrate;
        emit stateChanged();

        m_pump2->start();
    }

    else if (m_previousState == State::Mixing)
    {
        m_mixer->start();

        if (!m_mixer->isRunning())
        {
            return;
        }

        m_state = State::Mixing;
        emit stateChanged();
    }
    else if (m_previousState == State::Transferring)
    {
        if (m_pump3->state() == Pump::State::Fault)
        {
            return;
        }
        m_valve3->open();

        if (!m_valve3->isOpen())
        {
            return;
        }

        m_state = State::Transferring;
        emit stateChanged();

        m_pump3->start();
    }

    else if (m_previousState == State::TemperatureCheck)
    {
        m_state = State::TemperatureCheck;
        emit stateChanged();

        if (m_mixingTank->temperature() >= MinMixingTemperature &&
            m_mixingTank->temperature() <= MaxMixingTemperature)
        {
            startMixing();
        }
    }

}

void BatchController::onMixerStateChanged()
{
    if (!m_mixer)
    {
        return;
    }

    if (m_mixer->state() != Mixer::State::Fault)
    {
        return;
    }

    if (m_state == State::Idle ||
        m_state == State::Complete ||
        m_state == State::Paused)
    {
        return;
    }

    m_pump1->stop();
    m_pump2->stop();
    m_pump3->stop();

    m_valve1->close();
    m_valve2->close();
    m_valve3->close();

    m_previousState = m_state;
    m_state = State::Paused;

    emit stateChanged();
}

void BatchController::onPump1StateChanged()
{
    if (!m_pump1)
    {
        return;
    }

    if (m_pump1->state() != Pump::State::Fault)
    {
        return;
    }

    if (m_state == State::Idle ||
        m_state == State::Complete ||
        m_state == State::Paused)
    {
        return;
    }

    m_pump1->stop();
    m_valve1->close();

    m_previousState = m_state;
    m_stopReason = StopReason::Pump1Fault;
    m_state = State::Paused;

    emit stateChanged();
}
void BatchController::onPump2StateChanged()
{
    if (!m_pump2)
    {
        return;
    }

    if (m_pump2->state() != Pump::State::Fault)
    {
        return;
    }

    if (m_state == State::Idle ||
        m_state == State::Complete ||
        m_state == State::Paused)
    {
        return;
    }

    m_pump1->stop();
    m_pump2->stop();
    m_pump3->stop();

    m_valve1->close();
    m_valve2->close();
    m_valve3->close();

    m_mixer->stop();

    m_previousState = m_state;
    m_stopReason = StopReason::Pump2Fault;
    m_state = State::Paused;

    emit stateChanged();
}

void BatchController::onPump3StateChanged()
{
    if (!m_pump3 || m_pump3->state() != Pump::State::Fault)
    {
        return;
    }

    if (m_state != State::Transferring)
    {
        return;
    }

    m_pump1->stop();
    m_pump2->stop();
    m_pump3->stop();

    m_valve1->close();
    m_valve2->close();
    m_valve3->close();
    m_mixer->stop();

    m_previousState = m_state;
    m_stopReason = StopReason::Pump3Fault;
    m_state = State::Paused;
    emit stateChanged();
}