#ifndef RUNREPOSITORY_H
#define RUNREPOSITORY_H

#include <QSqlDatabase>
#include <QString>
#include <QtGlobal>

class RunRepository
{
public:
    explicit RunRepository(const QSqlDatabase &database);

    bool beginObservedBatch(const QString &runId, qint64 firstObservedAtMs, QString *errorMessage);
    bool beginBatch(const QString &runId, const QString &operatorId, qint64 startedAtMs, QString *errorMessage);
    bool beginDrain(const QString &runId,  qint64 startedAtMs, QString *errorMessage);
    bool finishRun(const QString &runId, const QString &outcome, qint64 endedAtMs, QString *errorMessage);
    bool runExists(const QString &runId, bool *exists, QString *errorMessage);
    bool hasConflictingCompletion(const QString &runId, const QString &expectedOutcome, bool *conflict, QString *errorMessage);

private:
    QSqlDatabase m_database;
};

#endif