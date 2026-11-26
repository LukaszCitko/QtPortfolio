#include "demomode.h"

#include "batchcontroller.h"
#include "processsimulator.h"

#include <QObject>
#include <QTimer>

void configureDemoMode(bool enabled, ProcessSimulator &simulator, BatchController &controller, QTimer &batchTimer)
{
    simulator.setEnabled(enabled);

    if (!enabled)
        return;

    constexpr int intervalMs = 100;
    constexpr double stepSeconds = intervalMs / 1000.0;

    QObject::connect(&batchTimer, &QTimer::timeout, &controller, [&controller]() { controller.simulateStep(stepSeconds); });
    batchTimer.start(intervalMs);
}