
#include "processsimulator.h"

#include "mixingtank.h"
#include "pump.h"
#include "valve.h"
#include <algorithm>

ProcessSimulator::ProcessSimulator(
    Pump *pump1,
    Valve *valve1,
    Pump *pump2,
    Valve *valve2,
    Pump *pump3,
    Valve *valve3,
    MixingTank *mixingTank,
    QObject *parent)
    : QObject(parent),
    m_pump1(pump1),
    m_valve1(valve1),
    m_pump2(pump2),
    m_valve2(valve2),
    m_pump3(pump3),
    m_valve3(valve3),
    m_mixingTank(mixingTank)
{
    Q_ASSERT(m_pump1);
    Q_ASSERT(m_valve1);
    Q_ASSERT(m_pump2);
    Q_ASSERT(m_valve2);
    Q_ASSERT(m_pump3);
    Q_ASSERT(m_valve3);
    Q_ASSERT(m_mixingTank);

    m_timer.setInterval(100);

    connect(&m_timer, &QTimer::timeout, this, &ProcessSimulator::updateSimulation);

    m_timer.start();
}

void ProcessSimulator::updateSimulation()
{
    simulateStep(m_timer.interval() / 1000.0);
}


    void ProcessSimulator::simulateStep(double elapsedSeconds)
{
    if (elapsedSeconds <= 0.0)
        return;

    if (!m_pump1 || !m_valve1 ||
        !m_pump2 || !m_valve2 ||
        !m_pump3 || !m_valve3 ||
        !m_mixingTank)
    {
        return;
    }

    // Remember the process conditions at the beginning
    // of this simulation step.

    const bool waterFlowActive =
        m_pump1->isRunning() && m_valve1->isOpen();

    const bool concentrateFlowActive =
        m_pump2->isRunning() && m_valve2->isOpen();


    // Simulate pump 1 RPM.

    auto updateActualRpm = [elapsedSeconds](Pump *pump) {
        const double requestedRpm =
            pump->isRunning() ? pump->targetRpm() : 0.0;

        const double maxChange = RpmChangePerSecond * elapsedSeconds;
        const double difference = requestedRpm - pump->actualRpm();
        const double change = std::clamp(difference, -maxChange, maxChange);

        pump->setActualRpm(pump->actualRpm() + change);
    };

    updateActualRpm(m_pump1.data());
    updateActualRpm(m_pump2.data());
    updateActualRpm(m_pump3.data());

    // Apply water flow using the conditions
    // from the beginning of this step.
    if (waterFlowActive)
    {
        const double waterAmount =
            WaterFlowPerSecond * elapsedSeconds
            * m_pump1->actualRpm() / NominalRpm;

        m_mixingTank->addWater(waterAmount);
    }

    // Apply concentrate flow using the conditions
    // from the beginning of this step.
    if (concentrateFlowActive)
    {
        const double concentrateAmount =
            ConcentrateFlowPerSecond * elapsedSeconds
            * m_pump2->actualRpm() / NominalRpm;

        m_mixingTank->addConcentrate(concentrateAmount);
    }
    // Simulate pump 2 RPM.
    const bool productFlowActive = m_pump3->isRunning() && m_valve3->isOpen();

    if (productFlowActive)
    {
        const double productAmount =
            ProductFlowPerSecond * elapsedSeconds
            * m_pump3->actualRpm() / NominalRpm;

        m_mixingTank->removeProduct(productAmount);
    }
}
