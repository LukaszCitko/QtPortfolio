#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QList>
#include <QString>

class UserListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ModelRole {
        IdRole = Qt::UserRole + 1,
        NameRole,
        AccessRole
    };

    explicit UserListModel(const QSqlDatabase &database,
                           QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool reload(QString *errorMessage);

private:
    struct User {
        QString id;
        QString displayName;
        QString role;
    };

    QSqlDatabase m_database;
    QList<User> m_users;
};

#endif