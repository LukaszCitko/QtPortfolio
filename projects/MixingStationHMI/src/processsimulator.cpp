
#include "processconfiguration.h"
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

    m_timer.setInterval(ProcessConfiguration::SimulationStepMilliseconds);

    connect(&m_timer, &QTimer::timeout, this, &ProcessSimulator::updateSimulation);

    m_timer.start();
}

void ProcessSimulator::setEnabled(bool enabled)
{
    if (enabled)
    {
        if (!m_timer.isActive())
        {
            m_timer.start();
        }

        return;
    }

    m_timer.stop();
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
        const double requestedRpm = pump->isRunning() ? pump->targetRpm() : 0.0;

        const double maxChange = static_cast<double>(ProcessConfiguration::PumpRpmChangePerSecond) * elapsedSeconds;
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
        const double waterFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::WaterFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
        const double waterAmount = waterFlowLitresPerSecond * elapsedSeconds * m_pump1->actualRpm() / Pump::MaxRpm;

        const double waterTargetLitres = static_cast<double>(ProcessConfiguration::WaterTargetDecilitres) / 10.0;
        const double remainingWaterLitres = std::max(0.0, waterTargetLitres - m_mixingTank->waterVolume());
        const double acceptedWaterAmount = std::min(waterAmount, remainingWaterLitres);

        m_mixingTank->addWater(acceptedWaterAmount);
    }

    // Apply concentrate flow using the conditions
    // from the beginning of this step.
    if (concentrateFlowActive)
    {
        const double concentrateFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::ConcentrateFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
        const double concentrateAmount = concentrateFlowLitresPerSecond * elapsedSeconds * m_pump2->actualRpm() / Pump::MaxRpm;

        const double concentrateTargetLitres = static_cast<double>(ProcessConfiguration::ConcentrateTargetDecilitres) / 10.0;
        const double remainingConcentrateLitres = std::max(0.0, concentrateTargetLitres - m_mixingTank->concentrateVolume());
        const double acceptedConcentrateAmount = std::min(concentrateAmount, remainingConcentrateLitres);

        m_mixingTank->addConcentrate(acceptedConcentrateAmount);
    }
    // Simulate pump 2 RPM.
    const bool productFlowActive = m_pump3->isRunning() && m_valve3->isOpen();

    if (productFlowActive)
    {
        const double productFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::ProductFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
        const double productAmount = productFlowLitresPerSecond * elapsedSeconds * m_pump3->actualRpm() / Pump::MaxRpm;

        m_mixingTank->removeProduct(productAmount);
    }
}
