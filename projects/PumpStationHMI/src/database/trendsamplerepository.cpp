#include "trendsamplerepository.h"

#include <QSqlError>
#include <QSqlQuery>

TrendSampleRepository::TrendSampleRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

bool TrendSampleRepository::saveSample(const QString &runId,
                                       qint64 timestampMs,
                                       double volumeL,
                                       double temperatureC,
                                       QString *errorMessage)
{
    if (runId.isEmpty()) {
        *errorMessage = "Run ID is required for a trend sample";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "INSERT INTO trend_samples "
            "(run_id, timestamp_ms, volume_l, temperature_c) "
            "VALUES (:run_id, :timestamp_ms, :volume_l, :temperature_c)")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);
    query.bindValue(":timestamp_ms", timestampMs);
    query.bindValue(":volume_l", volumeL);
    query.bindValue(":temperature_c", temperatureC);

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    return true;
}