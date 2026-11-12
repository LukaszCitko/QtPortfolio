#ifndef PERSISTENCERETRYQUEUE_H
#define PERSISTENCERETRYQUEUE_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QString>
#include <QtGlobal>
#include <QList>

class RunRepository;
class EventRepository;
class TrendSampleRepository;

class PersistenceRetryQueue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasPendingWrites READ hasPendingWrites NOTIFY hasPendingWritesChanged)
    Q_PROPERTY(bool hasPermanentFailure READ hasPermanentFailure NOTIFY hasPermanentFailureChanged)

public:
    explicit PersistenceRetryQueue(RunRepository &runs, EventRepository &events, TrendSampleRepository &samples, QObject *parent = nullptr);
    enum class RunStartKind { Batch, ObservedBatch, Drain };
    void submitRunStart(RunStartKind kind, const QString &runId, const QString &operatorId, qint64 timestampMs);
    void submitEvent(const QString &runId, qint64 timestampMs, const QString &level, const QString &source, const QString &message);
    void submitSample(const QString &runId, qint64 timestampMs, double volumeL, double temperatureC);
    void submitRunFinish(const QString &runId, const QString &outcome, qint64 timestampMs);
    bool hasPendingWrites() const;
    bool hasPermanentFailure() const;


signals:
    void eventSaved(const QString &runId, qint64 timestampMs, const QString &level, const QString &source, const QString &message);
    void hasPendingWritesChanged();
    void hasPermanentFailureChanged();

private:
    RunRepository &m_runs;
    EventRepository &m_events;
    TrendSampleRepository &m_samples;
    QTimer m_retryTimer;
    bool m_hasPendingWrites = false;
    bool m_hasPermanentFailure = false;
    void reportPermanentFailure(const QString &message);

    struct PendingRunStart {
        RunStartKind kind;
        QString runId;
        QString operatorId;
        qint64 timestampMs;
    };

    struct PendingEvent {
        QString runId;
        qint64 timestampMs;
        QString level;
        QString source;
        QString message;
    };

    struct PendingSample {
        QString runId;
        qint64 timestampMs;
        double volumeL;
        double temperatureC;
    };

    struct PendingRunFinish {
        QString runId;
        QString outcome;
        qint64 timestampMs;
    };

    QHash<QString, PendingRunStart> m_pendingRunStarts;
    QList<PendingEvent> m_pendingEvents;
    QList<PendingSample> m_pendingSamples;
    QHash<QString, PendingRunFinish> m_pendingRunFinishes;
    void retryRunFinishes();
    void retrySamples();
    void retryEvents();
    void updateRetryTimer();
    void retryRunStarts();
};

#endif