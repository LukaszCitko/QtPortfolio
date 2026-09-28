#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "trendhistorysource.h"

TrendHistorySource::TrendHistorySource(const QSqlDatabase &database,    QObject *parent)
    : QObject(parent),
    m_database(database)
{
}

int TrendHistorySource::sampleCount() const
{
    return static_cast<int>(m_samples.size());
}

QString TrendHistorySource::runId() const
{
    return m_runId;
}

QString TrendHistorySource::runIdAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_runId : QString();
}

qint64 TrendHistorySource::timestampMsAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).timestampMs : 0;
}

double TrendHistorySource::volumeAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).volumeL : 0.0;
}

double TrendHistorySource::temperatureAt(int index) const
{
    return index >= 0 && index < m_samples.size()
    ? m_samples.at(index).temperatureC : 0.0;
}

bool TrendHistorySource::loadRun(const QString &runId)
{
    if (runId.isEmpty()) {
        m_samples.clear();
        m_runId.clear();
        emit runChanged();
        emit samplesChanged();
        return true;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "SELECT timestamp_ms, volume_l, temperature_c "
            "FROM trend_samples "
            "WHERE run_id = :run_id "
            "ORDER BY timestamp_ms, sample_id")) {
        qWarning() << "Cannot prepare trend history query:"
                   << query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);

    if (!query.exec()) {
        qWarning() << "Cannot load trend history:"
                   << query.lastError().text();
        return false;
    }

    QList<Sample> loadedSamples;

    while (query.next()) {
        loadedSamples.append(Sample{
            query.value(0).toLongLong(),
            query.value(1).toDouble(),
            query.value(2).toDouble()
        });
    }

    const bool selectedRunChanged = m_runId != runId;
    m_runId = runId;
    m_samples = loadedSamples;

    if (selectedRunChanged)
        emit runChanged();

    emit samplesChanged();
    return true;
}