#include "demomode.h"

#include "batchcontroller.h"
#include "processsimulator.h"
#include "processeventcoordinator.h"

#include "database/runpersistencecoordinator.h"

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

void connectLocalBatchObservers(BatchController &controller, MixingTank &tank, OperatorSession &session, RunPersistenceCoordinator &runPersistence, ProcessEventCoordinator &processEvents, TrendRecorder &trends)
{
    runPersistence.watchBatchStart(controller, session);
    runPersistence.watchTrendRecording(controller, trends);
    processEvents.watchStageCompletion(controller);
    processEvents.watchBatchState(controller, tank, session);
    processEvents.watchBatchId(controller);
    runPersistence.watchBatchFinish(controller);
}