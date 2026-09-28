#ifndef TRENDRECORDER_H
#define TRENDRECORDER_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>

class MixingTank;

class TrendRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY samplesChanged)
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)

public:
    explicit TrendRecorder(MixingTank *tank, QObject *parent = nullptr);

    int sampleCount() const;
    bool isRecording() const;

    Q_INVOKABLE QString runIdAt(int index) const;
    Q_INVOKABLE qint64 timestampMsAt(int index) const;
    Q_INVOKABLE double volumeAt(int index) const;
    Q_INVOKABLE double temperatureAt(int index) const;

    void beginRun(const QString &runId = QString());
    void endRun();
    void recordNow();

signals:
    void samplesChanged();
    void recordingChanged();

    void sampleRecorded(const QString &runId,
                        qint64 timestampMs,
                        double volumeL,
                        double temperatureC);

private:
    struct Sample
    {
        QString runId;
        qint64 timestampMs;
        double volumeL;
        double temperatureC;
    };

    static constexpr int MaxLiveSamples = 600;

    QPointer<MixingTank> m_tank;
    QTimer m_timer;
    QList<Sample> m_samples;
    QString m_activeRunId;
    bool m_recording = false;
};

#endif