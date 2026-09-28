#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/userlistmodel.h"
#include "../src/database/userrepository.h"
#include "../src/eventmanager.h"
#include "../src/operatorsession.h"
#include "../src/usermanagementservice.h"

class TestUserManagementService : public QObject
{
    Q_OBJECT

private slots:
    void allowsOnlyAdminToManageUsers();
};

void TestUserManagementService::allowsOnlyAdminToManageUsers()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery seed(database);
    QVERIFY(seed.exec(
        "INSERT INTO users (user_id, display_name, role) VALUES "
        "('admin-1', 'Test Admin', 'ADMIN'), "
        "('operator-1', 'Test Operator', 'OPERATOR')"));

    UserRepository repository(database);
    UserListModel users(database);
    QVERIFY2(users.reload(&error), qPrintable(error));

    OperatorSession session(database);
    EventManager events;
    UserManagementService service(session, repository, users, events);

    QVERIFY(session.selectOperator("operator-1"));
    QVERIFY(!service.addUser("New Tech", "TECHNICIAN"));
    QCOMPARE(users.rowCount(), 2);

    QVERIFY(session.selectOperator("admin-1"));
    QVERIFY(service.addUser("  New Tech  ", "technician"));
    QCOMPARE(users.rowCount(), 3);

    QSqlQuery added(database);
    QVERIFY(added.exec(
        "SELECT role FROM users WHERE display_name = 'New Tech'"));
    QVERIFY(added.next());
    QCOMPARE(added.value(0).toString(), QStringLiteral("TECHNICIAN"));

    QVERIFY(service.changeRole("operator-1", "TECHNICIAN"));
    QVERIFY(!service.changeRole("admin-1", "OPERATOR"));

    QSqlQuery roles(database);
    QVERIFY(roles.exec(
        "SELECT user_id, role FROM users "
        "WHERE user_id IN ('admin-1', 'operator-1') "
        "ORDER BY user_id"));
    QVERIFY(roles.next());
    QCOMPARE(roles.value(0).toString(), QStringLiteral("admin-1"));
    QCOMPARE(roles.value(1).toString(), QStringLiteral("ADMIN"));
    QVERIFY(roles.next());
    QCOMPARE(roles.value(0).toString(), QStringLiteral("operator-1"));
    QVERIFY(!service.removeUser("admin-1"));
    QVERIFY(service.removeUser("operator-1"));
    QCOMPARE(users.rowCount(), 2);
    QVERIFY(!session.selectOperator("operator-1"));

    QSqlQuery removed(database);
    QVERIFY(removed.exec(
        "SELECT is_active FROM users WHERE user_id = 'operator-1'"));
    QVERIFY(removed.next());
    QCOMPARE(removed.value(0).toInt(), 0);

    QCOMPARE(roles.value(1).toString(), QStringLiteral("TECHNICIAN"));
}

QTEST_GUILESS_MAIN(TestUserManagementService)
#include "tst_usermanagementservice.moc"
