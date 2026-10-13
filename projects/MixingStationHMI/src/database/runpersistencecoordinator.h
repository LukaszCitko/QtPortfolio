#ifndef RUNPERSISTENCECOORDINATOR_H
#define RUNPERSISTENCECOORDINATOR_H

#include <QString>

class RunRepository;
class EventManager;
class BatchController;
class OperatorSession;
class TrendRecorder;
class RemoteProcessController;

class RunPersistenceCoordinator
{
public:
    RunPersistenceCoordinator(RunRepository &runs, EventManager &events);

    void watchBatchStart(BatchController &controller, OperatorSession &session);
    void watchBatchStart(RemoteProcessController &controller, OperatorSession &session);

    void watchBatchFinish(BatchController &controller);
    void watchBatchFinish(RemoteProcessController &controller);

    void watchTrendRecording(BatchController &controller, TrendRecorder &recorder);
    void watchTrendRecording(RemoteProcessController &controller, TrendRecorder &recorder);

    void watchControllerRecovery(RemoteProcessController &controller);
private:
    RunRepository &m_runs;
    EventManager &m_events;
    QString m_standaloneDrainRunId;
};

#endif