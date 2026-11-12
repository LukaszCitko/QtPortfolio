#ifndef RUNPERSISTENCECOORDINATOR_H
#define RUNPERSISTENCECOORDINATOR_H

#include <QString>

class EventManager;
class BatchController;
class OperatorSession;
class TrendRecorder;
class RemoteProcessController;
class PersistenceRetryQueue;
class RunPersistenceCoordinator
{
public:
    RunPersistenceCoordinator(EventManager &events, PersistenceRetryQueue &retryQueue);

    void watchBatchStart(BatchController &controller, OperatorSession &session);
    void watchBatchStart(RemoteProcessController &controller, OperatorSession &session);

    void watchBatchFinish(BatchController &controller);
    void watchBatchFinish(RemoteProcessController &controller);

    void watchTrendRecording(BatchController &controller, TrendRecorder &recorder);
    void watchTrendRecording(RemoteProcessController &controller, TrendRecorder &recorder);

    void watchControllerRecovery(RemoteProcessController &controller);

private:
    EventManager &m_events;
    QString m_standaloneDrainRunId;
    PersistenceRetryQueue &m_retryQueue;
};

#endif