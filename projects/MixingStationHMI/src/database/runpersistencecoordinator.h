#ifndef RUNPERSISTENCECOORDINATOR_H
#define RUNPERSISTENCECOORDINATOR_H

#include <QString>

class RunRepository;
class EventManager;
class BatchController;
class OperatorSession;
class TrendRecorder;

class RunPersistenceCoordinator
{
public:
    RunPersistenceCoordinator(RunRepository &runs, EventManager &events);

    void watchBatchStart(BatchController &controller, OperatorSession &session);
    void watchBatchFinish(BatchController &controller);
    void watchTrendRecording(BatchController &controller, TrendRecorder &recorder);

private:
    RunRepository &m_runs;
    EventManager &m_events;
    QString m_standaloneDrainRunId;
};

#endif