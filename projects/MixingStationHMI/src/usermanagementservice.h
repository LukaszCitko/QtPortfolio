#ifndef USERMANAGEMENTSERVICE_H
#define USERMANAGEMENTSERVICE_H

#include <QObject>
#include <QString>

class OperatorSession;
class UserRepository;
class UserListModel;
class EventManager;

class UserManagementService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    UserManagementService(OperatorSession &session,
                          UserRepository &repository,
                          UserListModel &users,
                          EventManager &events,
                          QObject *parent = nullptr);

    Q_INVOKABLE bool addUser(const QString &displayName,
                             const QString &role);
    Q_INVOKABLE bool changeRole(const QString &userId,
                                const QString &role);
    Q_INVOKABLE bool removeUser(const QString &userId);

    QString lastError() const;

signals:
    void lastErrorChanged();

private:
    void setLastError(const QString &message);

    OperatorSession &m_session;
    UserRepository &m_repository;
    UserListModel &m_users;
    EventManager &m_events;
    QString m_lastError;
};

#endif