#ifndef EVENTHISTORYMODEL_H
#define EVENTHISTORYMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QSqlDatabase>
#include <QString>

class EventHistoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString runFilter READ runFilter NOTIFY runFilterChanged)

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

    Q_INVOKABLE bool selectRun(const QString &runId);
    bool reload(QString *errorMessage);
    QString runFilter() const;
    void prependSavedEvent(const QString &runId,
                           qint64 timestampMs,
                           const QString &level,
                           const QString &source,
                           const QString &message);
signals:
    void runFilterChanged();
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
    QString m_runFilter;
};

#endif