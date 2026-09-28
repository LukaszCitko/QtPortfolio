#include "runpersistencecoordinator.h"

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QUuid>

#include "../trendrecorder.h"
#include "../batchcontroller.h"
#include "../eventmanager.h"
#include "../operatorsession.h"
#include "runrepository.h"

RunPersistenceCoordinator::RunPersistenceCoordinator(
    RunRepository &runs, EventManager &events)
    : m_runs(runs),
    m_events(events)
{
}

void RunPersistenceCoordinator::watchBatchStart(
    BatchController &controller, OperatorSession &session)
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