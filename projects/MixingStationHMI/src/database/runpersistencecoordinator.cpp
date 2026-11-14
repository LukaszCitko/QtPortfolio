#include "runpersistencecoordinator.h"
#include "persistenceretryqueue.h"

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QUuid>

#include "../trendrecorder.h"
#include "../batchcontroller.h"
#include "../eventmanager.h"
#include "../operatorsession.h"
#include "controllerprotocol.h"
#include "runrepository.h"
#include "../communication/remoteprocesscontroller.h"

RunPersistenceCoordinator::RunPersistenceCoordinator(EventManager &events, PersistenceRetryQueue &retryQueue)
    : m_events(events), m_retryQueue(retryQueue)
{
}

void RunPersistenceCoordinator::watchBatchStart(BatchController &controller, OperatorSession &session)
{
    BatchController *controllerPtr = &controller;
    OperatorSession *sessionPtr = &session;
    PersistenceRetryQueue *retryQueue = &m_retryQueue;

    QObject::connect(controllerPtr, &BatchController::batchIdChanged, sessionPtr, [controllerPtr, sessionPtr, retryQueue]() {
        const QString runId = controllerPtr->batchId();
        if (runId.isEmpty()) return;
        retryQueue->submitRunStart(PersistenceRetryQueue::RunStartKind::Batch, runId, sessionPtr->operatorId(), QDateTime::currentMSecsSinceEpoch());
        }
    );
}

void RunPersistenceCoordinator::watchBatchFinish(BatchController &controller)
{
    BatchController *controllerPtr = &controller;
    PersistenceRetryQueue *retryQueue = &m_retryQueue;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &BatchController::stateChanged, events,
                    [this, controllerPtr, retryQueue, events, previousState = controllerPtr->state()]() mutable
                    {
                        using State = BatchController::State;

                        const State currentState = controllerPtr->state();

                        if (currentState == previousState) return;

                        const State oldState = previousState;
                        previousState = currentState;

                        QString outcome;

                        if (currentState == State::Complete) {
                            outcome = "COMPLETED";
                        } else if (oldState == State::Draining
                                && currentState == State::Idle) {
                            outcome = "DRAINED";
                        } else {
                            return;
                        }

                        const bool standaloneDrain =
                            outcome == "DRAINED"
                            && controllerPtr->batchId().isEmpty()
                            && !m_standaloneDrainRunId.isEmpty();

                        const QString runId = standaloneDrain
                                                   ? m_standaloneDrainRunId
                                                   : controllerPtr->batchId();

                        if (runId.isEmpty()) return;

                        retryQueue->submitRunFinish(runId, outcome, QDateTime::currentMSecsSinceEpoch());

                        if (standaloneDrain) {
                            events->setBatchId(QString());
                            m_standaloneDrainRunId.clear();
                        }
                    }
    );
}

void RunPersistenceCoordinator::watchTrendRecording(BatchController &controller, TrendRecorder &recorder)
{
    BatchController *controllerPtr = &controller;
    TrendRecorder *recorderPtr = &recorder;

    QObject::connect(controllerPtr, &BatchController::stateChanged, &m_events,
                    [this, controllerPtr, recorderPtr, previousState = controllerPtr->state()]() mutable
                    {
                        using State = BatchController::State;

                        const State currentState = controllerPtr->state();

                        if (currentState == previousState)
                            return;

                        const State oldState = previousState;
                        previousState = currentState;

                        if (oldState == State::Idle
                            && currentState == State::FillingWater) {
                            recorderPtr->beginRun(controllerPtr->batchId());
                            return;
                        }

                        if (currentState == State::Draining
                            && !recorderPtr->isRecording()) {
                            QString runId = controllerPtr->batchId();

                            if (runId.isEmpty()) {
                                runId = QUuid::createUuid().toString(QUuid::WithoutBraces);

                                m_retryQueue.submitRunStart(PersistenceRetryQueue::RunStartKind::Drain, runId, QString(), QDateTime::currentMSecsSinceEpoch());

                                m_standaloneDrainRunId = runId;
                                m_events.setBatchId(runId);
                            }

                            recorderPtr->beginRun(runId);
                            return;
                        }

                        if (currentState == State::Complete
                                || (oldState == State::Draining
                                && currentState == State::Idle)) {
                                    recorderPtr->endRun();
                        }
                    }
    );
}

void RunPersistenceCoordinator::watchBatchStart(RemoteProcessController &controller, OperatorSession &session)
{
    RemoteProcessController *controllerPtr = &controller;
    OperatorSession *sessionPtr = &session;
    PersistenceRetryQueue *retryQueue = &m_retryQueue;

    QObject::connect(controllerPtr, &RemoteProcessController::batchIdChanged, sessionPtr, [controllerPtr, sessionPtr, retryQueue]() {
        const QString runId = controllerPtr->batchId();
        if (runId.isEmpty()) return;

        const int drainRunKind = static_cast<int>(ControllerProtocol::RunKind::Drain);
        const QString operatorId = sessionPtr->operatorId();
        const qint64 firstObservedAtMs = QDateTime::currentMSecsSinceEpoch();

        if (controllerPtr->runKind() == drainRunKind) {
            retryQueue->submitRunStart(PersistenceRetryQueue::RunStartKind::Drain, runId, QString(), firstObservedAtMs);
        } else if (operatorId.isEmpty()) {
            retryQueue->submitRunStart(PersistenceRetryQueue::RunStartKind::ObservedBatch, runId, QString(), firstObservedAtMs);
        } else {
            retryQueue->submitRunStart(PersistenceRetryQueue::RunStartKind::Batch, runId, operatorId, firstObservedAtMs);
        }
    });
}

void RunPersistenceCoordinator::watchBatchFinish(RemoteProcessController &controller)
{
    RemoteProcessController *controllerPtr = &controller;
    PersistenceRetryQueue *retryQueue = &m_retryQueue;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::stateChanged, events, [controllerPtr, retryQueue, previousState = controllerPtr->state()]() mutable
                    {
                        const int currentState = controllerPtr->state();
                        const int oldState = previousState;

                        if (currentState == oldState)
                        {
                            return;
                        }

                        previousState = currentState;

                        const int completeState = static_cast<int>(ControllerProtocol::ProcessState::Complete);

                        if (currentState != completeState)
                        {
                            return;
                        }

                        const QString runId = controllerPtr->batchId();

                        if (runId.isEmpty())
                        {
                            return;
                        }
                        const int drainRunKind = static_cast<int>(ControllerProtocol::RunKind::Drain);
                        const QString outcome = controllerPtr->runKind() == drainRunKind ? QStringLiteral("DRAINED") : QStringLiteral("COMPLETED");
                        retryQueue->submitRunFinish(runId, outcome, QDateTime::currentMSecsSinceEpoch());
                    }
    );
}

void RunPersistenceCoordinator::watchTrendRecording(RemoteProcessController &controller, TrendRecorder &recorder)
{
    RemoteProcessController *controllerPtr = &controller;
    TrendRecorder *recorderPtr = &recorder;

    QObject::connect(controllerPtr, &RemoteProcessController::batchIdChanged, recorderPtr, [controllerPtr, recorderPtr]()
                    {
                        const QString runId = controllerPtr->batchId();
                        const int completeState = static_cast<int>(ControllerProtocol::ProcessState::Complete);

                        if (runId.isEmpty() || controllerPtr->state() == completeState || recorderPtr->isRecording())
                        {
                            return;
                        }

                        recorderPtr->beginRun(runId);
                    }
    );

    QObject::connect(controllerPtr, &RemoteProcessController::stateChanged, &m_events, [controllerPtr, recorderPtr, previousState = controllerPtr->state()]() mutable
                    {
                        const int currentState = controllerPtr->state();
                        const int oldState = previousState;

                        if (currentState == oldState)
                        {
                            return;
                        }

                        previousState = currentState;

                        const int idleState = static_cast<int>(ControllerProtocol::ProcessState::Idle);
                        const int fillingState = static_cast<int>(ControllerProtocol::ProcessState::FillingWater);
                        const int drainingState = static_cast<int>(ControllerProtocol::ProcessState::Draining);
                        const int completeState = static_cast<int>(ControllerProtocol::ProcessState::Complete);

                        if (oldState == idleState && (currentState == fillingState || currentState == drainingState))
                        {
                            const QString runId = controllerPtr->batchId();
                            if (runId.isEmpty()) return;
                            recorderPtr->beginRun(runId);
                            return;
                        }

                        if (currentState == completeState)
                        {
                            recorderPtr->endRun();
                        }
                    }
    );
}

void RunPersistenceCoordinator::watchControllerRecovery(RemoteProcessController &controller)
{
    RemoteProcessController *controllerPtr = &controller;
    PersistenceRetryQueue *retryQueue = &m_retryQueue;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::recoveryRequiredChanged, events, [controllerPtr, retryQueue]()
                    {
                        if (!controllerPtr->recoveryRequired())
                        {
                            return;
                        }

                        const QString runId = controllerPtr->batchId();

                        if (runId.isEmpty())
                        {
                            return;
                        }

                        retryQueue->submitRunFinish(runId, QStringLiteral("INTERRUPTED"), QDateTime::currentMSecsSinceEpoch());
                    }
    );
}
