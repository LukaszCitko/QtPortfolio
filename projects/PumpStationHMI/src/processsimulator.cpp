
#include "processsimulator.h"

#include "mixingtank.h"
#include "pump.h"
#include "valve.h"

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

    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        &ProcessSimulator::updateSimulation);

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
    const double targetRpm1 = m_pump1->targetRpm();
    double actualRpm1 = m_pump1->actualRpm();

    if (actualRpm1 < targetRpm1)
    {
        actualRpm1 += RpmStep;

        if (actualRpm1 > targetRpm1)
            actualRpm1 = targetRpm1;

        m_pump1->setActualRpm(actualRpm1);
    }
    else if (actualRpm1 > targetRpm1)
    {
        actualRpm1 -= RpmStep;

        if (actualRpm1 < targetRpm1)
            actualRpm1 = targetRpm1;

        m_pump1->setActualRpm(actualRpm1);
    }

    // Simulate pump 2 RPM.
    const double targetRpm2 = m_pump2->targetRpm();
    double actualRpm2 = m_pump2->actualRpm();

    if (actualRpm2 < targetRpm2)
    {
        actualRpm2 += RpmStep;

        if (actualRpm2 > targetRpm2)
            actualRpm2 = targetRpm2;

        m_pump2->setActualRpm(actualRpm2);
    }
    else if (actualRpm2 > targetRpm2)
    {
        actualRpm2 -= RpmStep;

        if (actualRpm2 < targetRpm2)
            actualRpm2 = targetRpm2;

        m_pump2->setActualRpm(actualRpm2);
    }

    // Apply water flow using the conditions
    // from the beginning of this step.
    if (waterFlowActive)
    {
        const double waterAmount =
            WaterFlowPerSecond * elapsedSeconds;

        m_mixingTank->addWater(waterAmount);
    }

    // Apply concentrate flow using the conditions
    // from the beginning of this step.
    if (concentrateFlowActive)
    {
        const double concentrateAmount =
            ConcentrateFlowPerSecond * elapsedSeconds;

        m_mixingTank->addConcentrate(concentrateAmount);
    }
    // Simulate pump 2 RPM.
    const bool productFlowActive = m_pump3->isRunning() && m_valve3->isOpen();

    if (productFlowActive)
    {
        const double productAmount = ProductFlowPerSecond * elapsedSeconds; m_mixingTank->removeProduct(productAmount);
    }
}
