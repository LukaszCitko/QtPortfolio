#include "runpersistencecoordinator.h"

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

RunPersistenceCoordinator::RunPersistenceCoordinator(RunRepository &runs, EventManager &events)
    : m_runs(runs), m_events(events)
{
}

void RunPersistenceCoordinator::watchBatchStart(BatchController &controller, OperatorSession &session)
{
    BatchController *controllerPtr = &controller;
    OperatorSession *sessionPtr = &session;
    RunRepository *runs = &m_runs;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &BatchController::batchIdChanged, sessionPtr,
        [controllerPtr, sessionPtr, runs, events]()
        {
            const QString runId = controllerPtr->batchId();

            if (runId.isEmpty()) return;

            QString error;
            if (!runs->beginBatch(
                    runId,
                    sessionPtr->operatorId(),
                    QDateTime::currentMSecsSinceEpoch(),
                    &error)) {
                qCritical() << "Cannot save batch run:" << error;
                events->addAlarm("DATABASE", "Batch record could not be saved");
            }
        }
    );
}

void RunPersistenceCoordinator::watchBatchFinish(BatchController &controller)
{
    BatchController *controllerPtr = &controller;
    RunRepository *runs = &m_runs;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &BatchController::stateChanged, events,
                    [this, controllerPtr, runs, events, previousState = controllerPtr->state()]() mutable
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

                        QString error;
                        if (!runs->finishRun(runId, outcome, QDateTime::currentMSecsSinceEpoch(), &error)) {
                            qCritical() << "Cannot finish run:" << error;
                            events->addAlarm("DATABASE", "Run outcome could not be saved");
                        }

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

                                QString error;
                                if (!m_runs.beginDrain(runId, QDateTime::currentMSecsSinceEpoch(), &error)) {
                                    qCritical() << "Cannot save drain run:" << error;
                                    m_events.addAlarm("DATABASE", "Drain record could not be saved");
                                    return;
                                }

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
    RunRepository *runs = &m_runs;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::batchIdChanged, sessionPtr, [controllerPtr, sessionPtr, runs, events]()
                    {
                        const QString runId = controllerPtr->batchId();

                        if (runId.isEmpty())
                        {
                            return;
                        }

                        QString error;
                        bool exists = false;

                        if (!runs->runExists(runId, &exists, &error))
                        {
                            qCritical() << "Cannot check remote run:" << error;
                            events->addAlarm("DATABASE", "Remote run could not be checked");
                            return;
                        }

                        if (exists)
                        {
                            return;
                        }

                        const int drainRunKind = static_cast<int>(ControllerProtocol::RunKind::Drain);
                        const bool saved = controllerPtr->runKind() == drainRunKind
                                               ? runs->beginDrain(runId, QDateTime::currentMSecsSinceEpoch(), &error)
                                               : runs->beginBatch(runId, sessionPtr->operatorId(), QDateTime::currentMSecsSinceEpoch(), &error);

                        if (!saved)
                        {
                            qCritical() << "Cannot save remote run:" << error;
                            events->addAlarm("DATABASE", "Remote run record could not be saved");
                        }
                     }
    );
}

void RunPersistenceCoordinator::watchBatchFinish(RemoteProcessController &controller)
{
    RemoteProcessController *controllerPtr = &controller;
    RunRepository *runs = &m_runs;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::stateChanged, events, [controllerPtr, runs, events, previousState = controllerPtr->state()]() mutable
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
                        QString error;

                        if (!runs->finishRun(runId, outcome, QDateTime::currentMSecsSinceEpoch(), &error))
                        {
                            qCritical() << "Cannot finish remote run:" << error;
                            events->addAlarm("DATABASE", "Remote run outcome could not be saved");
                        }
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
                            recorderPtr->beginRun(controllerPtr->batchId());
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
    RunRepository *runs = &m_runs;
    EventManager *events = &m_events;

    QObject::connect(controllerPtr, &RemoteProcessController::recoveryRequiredChanged, events, [controllerPtr, runs, events]()
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

                        QString error;

                        if (!runs->finishRun(runId, QStringLiteral("INTERRUPTED"), QDateTime::currentMSecsSinceEpoch(), &error))
                        {
                            qCritical() << "Cannot mark interrupted run:" << error;
                            events->addAlarm(QStringLiteral("DATABASE"), QStringLiteral("Interrupted run could not be saved"));
                        }
                    }
    );
}
