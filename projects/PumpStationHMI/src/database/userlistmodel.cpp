#include "userlistmodel.h"
#include <QSqlError>
#include <QSqlQuery>

UserListModel::UserListModel(const QSqlDatabase &database, QObject *parent)
    : QAbstractListModel(parent),
    m_database(database)
{
}

int UserListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_users.size());
}

QVariant UserListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()
        || index.column() != 0
        || index.row() >= m_users.size()) {
        return {};
    }

    const User &user = m_users.at(index.row());

    switch (role) {
    case IdRole:
        return user.id;
    case NameRole:
    case Qt::DisplayRole:
        return user.displayName;
    case AccessRole:
        return user.role;
    default:
        return {};
    }
}

QHash<int, QByteArray> UserListModel::roleNames() const
{
    return {
        {IdRole, "userId"},
        {NameRole, "displayName"},
        {AccessRole, "role"}
    };
}

bool UserListModel::reload(QString *errorMessage)
{
    QSqlQuery query(m_database);

    if (!query.exec(
            "SELECT user_id, display_name, role "
            "FROM users "
            "WHERE is_active = 1 "
            "ORDER BY display_name, user_id")) {
        *errorMessage = query.lastError().text();
        return false;
    }

    QList<User> loadedUsers;

    while (query.next()) {
        loadedUsers.append(User{
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString()
        });
    }

    beginResetModel();
    m_users = loadedUsers;
    endResetModel();

    return true;
}