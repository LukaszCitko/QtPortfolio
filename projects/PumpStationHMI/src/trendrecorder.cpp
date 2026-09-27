#include "trendrecorder.h"

#include <QDateTime>
#include <QUuid>

#include "mixingtank.h"

TrendRecorder::TrendRecorder(MixingTank *tank, QObject *parent)
    : QObject(parent),
    m_tank(tank)
{
    m_timer.setInterval(1000);

    connect(&m_timer, &QTimer::timeout,
            this, &TrendRecorder::recordNow);
}

int TrendRecorder::sampleCount() const
{
    return m_samples.size();
}

bool TrendRecorder::isRecording() const
{
    return m_recording;
}

void TrendRecorder::beginRun(const QString &runId)
{
    if (m_recording || !m_tank)
        return;

    m_activeRunId = runId.isEmpty()
                        ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                        : runId;

    m_recording = true;
    emit recordingChanged();

    recordNow();
    m_timer.start();
}

void TrendRecorder::endRun()
{
    if (!m_recording)
        return;

    m_timer.stop();
    recordNow();

    m_recording = false;
    emit recordingChanged();
}

void TrendRecorder::recordNow()
{
    if (!m_recording || !m_tank)
        return;

    const Sample sample{
        m_activeRunId,
        QDateTime::currentMSecsSinceEpoch(),
        m_tank->volume(),
        m_tank->temperature()
    };

    if (m_samples.size() == MaxLiveSamples)
        m_samples.removeFirst();

    m_samples.append(sample);

    emit samplesChanged();
    emit sampleRecorded(sample.runId,
                        sample.timestampMs,
                        sample.volumeL,
                        sample.temperatureC);
}

QString TrendRecorder::runIdAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).runId : QString();
}

qint64 TrendRecorder::timestampMsAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).timestampMs : 0;
}

double TrendRecorder::volumeAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).volumeL : 0.0;
}

double TrendRecorder::temperatureAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).temperatureC : 0.0;
}