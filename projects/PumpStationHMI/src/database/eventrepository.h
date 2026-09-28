#ifndef EVENTREPOSITORY_H
#define EVENTREPOSITORY_H

#include <QSqlDatabase>
#include <QString>
#include <QtGlobal>

class EventRepository
{
public:
    explicit EventRepository(const QSqlDatabase &database);

    bool saveEvent(const QString &runId,
                   qint64 timestampMs,
                   const QString &level,
                   const QString &source,
                   const QString &message,
                   QString *errorMessage);

private:
    QSqlDatabase m_database;
};

#endif