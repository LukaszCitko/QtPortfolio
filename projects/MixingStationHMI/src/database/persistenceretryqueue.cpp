#include "persistenceretryqueue.h"

#include "eventrepository.h"
#include "runrepository.h"
#include "trendsamplerepository.h"

#include <QDebug>

PersistenceRetryQueue::PersistenceRetryQueue(RunRepository &runs, EventRepository &events, TrendSampleRepository &samples, QObject *parent)
    : QObject(parent), m_runs(runs), m_events(events), m_samples(samples)
{
    m_retryTimer.setInterval(2000);
    connect(&m_retryTimer, &QTimer::timeout, this, [this]() { retryRunStarts(); retryRunFinishes(); retryEvents(); retrySamples(); updateRetryTimer(); });
}

void PersistenceRetryQueue::submitRunStart(RunStartKind kind, const QString &runId, const QString &operatorId, qint64 timestampMs)
{
    if (runId.isEmpty() || (kind == RunStartKind::Batch && operatorId.isEmpty())) {
        qWarning() << "Invalid run start request:" << runId;
        return;
    }

    if (!m_pendingRunStarts.contains(runId)) m_pendingRunStarts.insert(runId, PendingRunStart{kind, runId, operatorId, timestampMs});
    retryRunStarts();
    updateRetryTimer();
}

void PersistenceRetryQueue::retryRunStarts()
{
    for (auto it = m_pendingRunStarts.begin(); it != m_pendingRunStarts.end();) {
        const PendingRunStart pending = it.value();
        QString error;

        bool exists = false;

        if (!m_runs.runExists(pending.runId, &exists, &error)) {
            ++it;
            continue;
        }

        if (exists) {
            it = m_pendingRunStarts.erase(it);
            continue;
        }

        bool saved = false;
        switch (pending.kind) {
        case RunStartKind::Batch:
            saved = m_runs.beginBatch(pending.runId, pending.operatorId, pending.timestampMs, &error);
            break;
        case RunStartKind::ObservedBatch:
            saved = m_runs.beginObservedBatch(pending.runId, pending.timestampMs, &error);
            break;
        case RunStartKind::Drain:
            saved = m_runs.beginDrain(pending.runId, pending.timestampMs, &error);
            break;
        }

        if (saved) {
            it = m_pendingRunStarts.erase(it);
        } else {
            ++it;
        }
    }
}

void PersistenceRetryQueue::submitEvent(const QString &runId, qint64 timestampMs, const QString &level, const QString &source, const QString &message)
{
    if (m_pendingEvents.size() >= MaxPendingEvents) {
        m_pendingEvents.removeFirst();
        if (!m_eventOverflowReported) {
            m_eventOverflowReported = true;
            reportPermanentFailure(QStringLiteral("Pending event limit exceeded; oldest events were discarded"));
        }
    }
    m_pendingEvents.append(PendingEvent{runId, timestampMs, level, source, message});
    retryEvents();
    updateRetryTimer();
}

void PersistenceRetryQueue::retryEvents()
{
    while (!m_pendingEvents.isEmpty()) {
        const PendingEvent pending = m_pendingEvents.first();
        QString error;

        if (!m_events.saveEvent(pending.runId, pending.timestampMs, pending.level, pending.source, pending.message, &error))
            break;

        m_pendingEvents.removeFirst();
        emit eventSaved(pending.runId, pending.timestampMs, pending.level, pending.source, pending.message);
    }
}

void PersistenceRetryQueue::updateRetryTimer()
{
    const bool pending = !m_pendingRunStarts.isEmpty() || !m_pendingRunFinishes.isEmpty() || !m_pendingEvents.isEmpty() || !m_pendingSamples.isEmpty();

    if (m_hasPendingWrites != pending) {
        m_hasPendingWrites = pending;
        emit hasPendingWritesChanged();
    }

    if (!pending)
        m_retryTimer.stop();
    else if (!m_retryTimer.isActive())
        m_retryTimer.start();
}

void PersistenceRetryQueue::submitSample(const QString &runId, qint64 timestampMs, double volumeL, double temperatureC)
{
    if (m_pendingSamples.size() >= MaxPendingSamples) {
        m_pendingSamples.removeFirst();
        if (!m_sampleOverflowReported) {
            m_sampleOverflowReported = true;
            reportPermanentFailure(QStringLiteral("Pending sample limit exceeded; oldest samples were discarded"));
        }
    }
    m_pendingSamples.append(PendingSample{runId, timestampMs, volumeL, temperatureC});
    retrySamples();
    updateRetryTimer();
}

void PersistenceRetryQueue::retrySamples()
{
    while (!m_pendingSamples.isEmpty()) {
        const PendingSample pending = m_pendingSamples.first();
        QString error;

        if (!m_samples.saveSample(pending.runId, pending.timestampMs, pending.volumeL, pending.temperatureC, &error))
            break;

        m_pendingSamples.removeFirst();
    }
}

void PersistenceRetryQueue::submitRunFinish(const QString &runId, const QString &outcome, qint64 timestampMs)
{
    if (runId.isEmpty() || outcome.isEmpty()) {
        qWarning() << "Invalid run finish request:" << runId << outcome;
        return;
    }

    const auto existing = m_pendingRunFinishes.constFind(runId);
    if (existing != m_pendingRunFinishes.constEnd()) {
        if (existing->outcome != outcome)
            reportPermanentFailure(QStringLiteral("Conflicting completion requests for run ") + runId);
    } else {
        m_pendingRunFinishes.insert(runId, PendingRunFinish{runId, outcome, timestampMs});
    }
    retryRunFinishes();
    updateRetryTimer();
}

void PersistenceRetryQueue::retryRunFinishes()
{
    for (auto it = m_pendingRunFinishes.begin(); it != m_pendingRunFinishes.end();) {
        const PendingRunFinish pending = it.value();
        QString error;

        if (m_runs.finishRun(pending.runId, pending.outcome, pending.timestampMs, &error)) {
            it = m_pendingRunFinishes.erase(it);
        } else {
            bool conflict = false;
            QString lookupError;

            if (m_runs.hasConflictingCompletion(pending.runId, pending.outcome, &conflict, &lookupError) && conflict) {
                reportPermanentFailure(QStringLiteral("Conflicting stored completion for run ") + pending.runId);
                it = m_pendingRunFinishes.erase(it);
            } else {
                ++it;
            }
        }
    }
}

bool PersistenceRetryQueue::hasPendingWrites() const
{
    return m_hasPendingWrites;
}

bool PersistenceRetryQueue::hasPermanentFailure() const
{
    return m_hasPermanentFailure;
}

void PersistenceRetryQueue::reportPermanentFailure(const QString &message)
{
    qCritical() << "History persistence failure:" << message;

    if (!m_hasPermanentFailure) {
        m_hasPermanentFailure = true;
        emit hasPermanentFailureChanged();
    }
}
