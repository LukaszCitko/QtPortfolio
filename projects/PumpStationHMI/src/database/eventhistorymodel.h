#ifndef EVENTHISTORYMODEL_H
#define EVENTHISTORYMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QSqlDatabase>
#include <QString>

class EventHistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ModelRole {
        RunIdRole = Qt::UserRole + 1,
        TimeRole,
        LevelRole,
        SourceRole,
        MessageRole
    };

    explicit EventHistoryModel(const QSqlDatabase &database,
                               QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool reload(QString *errorMessage);
    void prependSavedEvent(const QString &runId,
                           qint64 timestampMs,
                           const QString &level,
                           const QString &source,
                           const QString &message);
private:
    struct Event {
        QString runId;
        QString time;
        QString level;
        QString source;
        QString message;
    };

    QSqlDatabase m_database;
    QList<Event> m_events;
};

#endif