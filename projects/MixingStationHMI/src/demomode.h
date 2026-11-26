#ifndef DEMOMODE_H
#define DEMOMODE_H

class BatchController;
class MixingTank;
class OperatorSession;
class ProcessEventCoordinator;
class ProcessSimulator;
class RunPersistenceCoordinator;
class QTimer;
class TrendRecorder;

void configureDemoMode(bool enabled, ProcessSimulator &simulator, BatchController &controller, QTimer &batchTimer);
void connectLocalBatchObservers(BatchController &controller, MixingTank &tank, OperatorSession &session, RunPersistenceCoordinator &runPersistence, ProcessEventCoordinator &processEvents, TrendRecorder &trends);

#endif