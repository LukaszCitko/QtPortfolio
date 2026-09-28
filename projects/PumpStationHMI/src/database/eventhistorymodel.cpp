#include "eventhistorymodel.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>

EventHistoryModel::EventHistoryModel(const QSqlDatabase &database,
                                     QObject *parent)
    : QAbstractListModel(parent),
    m_database(database)
{
}

int EventHistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_events.size());
}

QVariant EventHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.column() != 0
        || index.row() < 0
        || index.row() >= m_events.size()) {
        return {};
    }

    const Event &event = m_events.at(index.row());

    switch (role) {
    case RunIdRole:  return event.runId;
    case TimeRole:   return event.time;
    case LevelRole:  return event.level;
    case SourceRole: return event.source;
    case MessageRole:
    case Qt::DisplayRole:
        return event.message;
    default:
        return {};
    }
}

QHash<int, QByteArray> EventHistoryModel::roleNames() const
{
    return {
        {RunIdRole, "runId"},
        {TimeRole, "time"},
        {LevelRole, "level"},
        {SourceRole, "source"},
        {MessageRole, "message"}
    };
}

bool EventHistoryModel::reload(QString *errorMessage)
{
    QSqlQuery query(m_database);

    if (!query.exec(
            "SELECT run_id, timestamp_ms, level, source, message "
            "FROM events "
            "ORDER BY event_id DESC")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    QList<Event> loadedEvents;

    while (query.next()) {
        const qint64 timestampMs = query.value(1).toLongLong();

        loadedEvents.append(Event{
            query.value(0).toString(),
            QDateTime::fromMSecsSinceEpoch(timestampMs)
                .toString("yyyy-MM-dd HH:mm:ss"),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toString()
        });
    }

    beginResetModel();
    m_events = loadedEvents;
    endResetModel();

    return true;
}
void EventHistoryModel::prependSavedEvent(const QString &runId,
                                          qint64 timestampMs,
                                          const QString &level,
                                          const QString &source,
                                          const QString &message)
{
    const Event event{
        runId,
        QDateTime::fromMSecsSinceEpoch(timestampMs)
            .toString("yyyy-MM-dd HH:mm:ss"),
        level,
        source,
        message
    };

    beginInsertRows(QModelIndex(), 0, 0);
    m_events.prepend(event);
    endInsertRows();
}