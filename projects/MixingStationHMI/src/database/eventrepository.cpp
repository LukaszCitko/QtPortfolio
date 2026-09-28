#include "eventrepository.h"

#include <QSqlError>
#include <QSqlQuery>

EventRepository::EventRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

bool EventRepository::saveEvent(const QString &runId,
                                qint64 timestampMs,
                                const QString &level,
                                const QString &source,
                                const QString &message,
                                QString *errorMessage)
{
    QSqlQuery query(m_database);

    if (!query.prepare(
            "INSERT INTO events "
            "(run_id, timestamp_ms, level, source, message) "
            "VALUES (NULLIF(:run_id, ''), :timestamp_ms, :level, :source, :message)")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);
    query.bindValue(":timestamp_ms", timestampMs);
    query.bindValue(":level", level);
    query.bindValue(":source", source);
    query.bindValue(":message", message);

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    return true;
}