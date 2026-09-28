#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

#include "runlistmodel.h"


RunListModel::RunListModel(const QSqlDatabase &database, QObject *parent)
    : QAbstractListModel(parent),
    m_database(database)
{
}

int RunListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return static_cast<int>(m_runs.size());
}

QVariant RunListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.column() != 0
        || index.row() < 0
        || index.row() >= m_runs.size()) {
        return {};
    }

    const Run &run = m_runs.at(index.row());

    switch (role) {
    case RunIdRole:       return run.runId;
    case KindRole:        return run.kind;
    case OperatorNameRole: return run.operatorName;
    case StartedAtRole:   return run.startedAt;
    case OutcomeRole:     return run.outcome;
    default:              return {};
    }
}

QHash<int, QByteArray> RunListModel::roleNames() const
{
    return {
        {RunIdRole, "runId"},
        {KindRole, "kind"},
        {OperatorNameRole, "operatorName"},
        {StartedAtRole, "startedAt"},
        {OutcomeRole, "outcome"}
    };
}

bool RunListModel::reload(QString *errorMessage)
{
    QSqlQuery query(m_database);

    if (!query.exec(
            "SELECT r.run_id, r.kind, "
            "COALESCE(u.display_name, ''), "
            "r.started_at_ms, COALESCE(r.outcome, '') "
            "FROM runs AS r "
            "LEFT JOIN users AS u "
            "ON u.user_id = r.batch_operator_user_id "
            "ORDER BY r.started_at_ms DESC, r.run_id DESC")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    QList<Run> loadedRuns;

    while (query.next()) {
        const qint64 startedAtMs = query.value(3).toLongLong();

        loadedRuns.append(Run{
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            QDateTime::fromMSecsSinceEpoch(startedAtMs)
                .toString("yyyy-MM-dd HH:mm:ss"),
            query.value(4).toString()
        });
    }

    beginResetModel();
    m_runs = loadedRuns;
    endResetModel();

    return true;
}

bool RunListModel::refresh()
{
    QString error;

    if (!reload(&error)) {
        qWarning() << "Cannot refresh run list:" << error;
        return false;
    }

    return true;
}