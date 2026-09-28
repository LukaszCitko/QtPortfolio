#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QSqlDatabase>
#include <QString>

class UserRepository
{
public:
    explicit UserRepository(const QSqlDatabase &database);

    bool addUser(const QString &displayName,
                 const QString &role,
                 QString *errorMessage);

    bool changeRole(const QString &userId,
                    const QString &role,
                    QString *errorMessage);
    bool deactivateUser(const QString &userId,
                        QString *errorMessage);

private:
    QSqlDatabase m_database;
};

#endif