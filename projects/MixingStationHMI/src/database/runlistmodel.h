#ifndef RUNLISTMODEL_H
#define RUNLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QSqlDatabase>
#include <QString>

class RunListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ModelRole {
        RunIdRole = Qt::UserRole + 1,
        KindRole,
        OperatorNameRole,
        StartedAtRole,
        OutcomeRole
    };

    explicit RunListModel(const QSqlDatabase &database,
                          QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool reload(QString *errorMessage);
    Q_INVOKABLE bool refresh();

private:
    struct Run {
        QString runId;
        QString kind;
        QString operatorName;
        QString startedAt;
        QString outcome;
    };

    QSqlDatabase m_database;
    QList<Run> m_runs;
};

#endif