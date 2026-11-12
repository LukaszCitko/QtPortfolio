#include "runrepository.h"

#include <QSqlError>
#include <QSqlQuery>

RunRepository::RunRepository(const QSqlDatabase &database)
    : m_database(database)
{
}

bool RunRepository::beginBatch(const QString &runId, const QString &operatorId, qint64 startedAtMs, QString *errorMessage)
{
    if (runId.isEmpty() || operatorId.isEmpty()) {
        *errorMessage = "Run ID and operator ID are required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "INSERT INTO runs "
            "(run_id, kind, batch_operator_user_id, started_at_ms) "
            "VALUES (:run_id, 'BATCH', :operator_id, :started_at_ms)")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);
    query.bindValue(":operator_id", operatorId);
    query.bindValue(":started_at_ms", startedAtMs);

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    return true;
}
bool RunRepository::beginObservedBatch(const QString &runId, qint64 firstObservedAtMs, QString *errorMessage)
{
    if (runId.isEmpty()) {
        *errorMessage = "Run ID is required";
        return false;
    }

    QSqlQuery query(m_database);
    if (!query.prepare("INSERT INTO runs (run_id, kind, started_at_ms) VALUES (:run_id, 'BATCH', :started_at_ms)")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);
    query.bindValue(":started_at_ms", firstObservedAtMs);
    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

bool RunRepository::finishRun(const QString &runId, const QString &outcome, qint64 endedAtMs, QString *errorMessage)
{
    if (runId.isEmpty() || outcome.isEmpty()) {
        *errorMessage = "Run ID and outcome are required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "UPDATE runs "
            "SET ended_at_ms = :ended_at_ms, outcome = :outcome "
            "WHERE run_id = :run_id AND ended_at_ms IS NULL")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":ended_at_ms", endedAtMs);
    query.bindValue(":outcome", outcome);
    query.bindValue(":run_id", runId);

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 1)
        return true;

    QSqlQuery existing(m_database);

    if (!existing.prepare("SELECT ended_at_ms, outcome FROM runs WHERE run_id = :run_id")) {
        *errorMessage = existing.lastError().text();
        return false;
    }

    existing.bindValue(":run_id", runId);

    if (!existing.exec()) {
        *errorMessage = existing.lastError().text();
        return false;
    }

    if (existing.next() && !existing.value(0).isNull() && existing.value(1).toString() == outcome)
        return true;

    *errorMessage = "No unfinished run with this outcome found for ID: " + runId;
    return false;

}

bool RunRepository::beginDrain(const QString &runId, qint64 startedAtMs, QString *errorMessage)
{
    if (runId.isEmpty()) {
        *errorMessage = "Run ID is required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare(
            "INSERT INTO runs (run_id, kind, started_at_ms) "
            "VALUES (:run_id, 'DRAIN', :started_at_ms)")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);
    query.bindValue(":started_at_ms", startedAtMs);

    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

bool RunRepository::runExists(const QString &runId, bool *exists, QString *errorMessage)
{
    if (runId.isEmpty())
    {
        *errorMessage = "Run ID is required";
        return false;
    }

    QSqlQuery query(m_database);

    if (!query.prepare("SELECT 1 FROM runs WHERE run_id = :run_id LIMIT 1"))
    {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);

    if (!query.exec())
    {
        *errorMessage = query.lastError().text();
        return false;
    }

    *exists = query.next();
    return true;
}

bool RunRepository::hasConflictingCompletion(const QString &runId, const QString &expectedOutcome, bool *conflict, QString *errorMessage)
{
    *conflict = false;

    QSqlQuery query(m_database);
    if (!query.prepare("SELECT outcome FROM runs WHERE run_id = :run_id AND ended_at_ms IS NOT NULL")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    query.bindValue(":run_id", runId);
    if (!query.exec()) {
        *errorMessage = query.lastError().text();
        return false;
    }

    if (query.next())
        *conflict = query.value(0).toString() != expectedOutcome;

    return true;
}
