#include "processsimulator.h"
#include "pump.h"
#include <algorithm>

ProcessSimulator::ProcessSimulator(Pump *pump, QObject *parent)
    : QObject{parent}
    , m_pump(pump)
{
    Q_ASSERT(pump != nullptr); // no pump no fun :)  TODO: when few pumps and vales added change dependency.
    m_timer.setInterval(100);

    connect(&m_timer, &QTimer::timeout, this, &ProcessSimulator::update);
    m_timer.start();
}

void ProcessSimulator::update()
{
    if (m_pump == nullptr)
    {
        return;
    }


    // leaving target const here but stil changing actual rpm
    const double effectiveTarget =
        m_pump->state() == Pump::State::Running
            ? m_pump->targetRpm()
            : 0.0;

    const double actual = m_pump->actualRpm();

    if (actual < effectiveTarget)
    {
        m_pump->setActualRpm(
            std::min(actual + RpmStep, effectiveTarget)
            );
    }
    else if (actual > effectiveTarget)
    {
        m_pump->setActualRpm(
            std::max(actual - RpmStep, effectiveTarget)
            );
    }
}