#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/userrepository.h"

class TestUserRepository : public QObject
{
    Q_OBJECT

private slots:
    void addsUserAndChangesRole();
};

void TestUserRepository::addsUserAndChangesRole()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    UserRepository repository(database);

    QVERIFY(!repository.addUser("   ", "OPERATOR", &error));
    QVERIFY2(repository.addUser("  Test User  ", "OPERATOR", &error),
             qPrintable(error));

    QSqlQuery user(database);
    QVERIFY(user.exec(
        "SELECT user_id, display_name, role, is_active FROM users"));
    QVERIFY(user.next());

    const QString userId = user.value(0).toString();
    QVERIFY(!userId.isEmpty());
    QCOMPARE(user.value(1).toString(), QStringLiteral("Test User"));
    QCOMPARE(user.value(2).toString(), QStringLiteral("OPERATOR"));
    QCOMPARE(user.value(3).toInt(), 1);
    QVERIFY(!user.next());

    QVERIFY2(repository.changeRole(userId, "TECHNICIAN", &error),
             qPrintable(error));
    QVERIFY(!repository.changeRole(userId, "INVALID", &error));
    QVERIFY(!repository.changeRole("missing-user", "ADMIN", &error));

    QSqlQuery updated(database);
    QVERIFY(updated.exec(
        "SELECT role FROM users WHERE user_id = '" + userId + "'"));
    QVERIFY(updated.next());
    QCOMPARE(updated.value(0).toString(), QStringLiteral("TECHNICIAN"));
    QVERIFY2(repository.deactivateUser(userId, &error), qPrintable(error));
    QVERIFY(!repository.deactivateUser(userId, &error));

    QSqlQuery deactivated(database);
    QVERIFY(deactivated.exec(
        "SELECT is_active FROM users WHERE user_id = '" + userId + "'"));
    QVERIFY(deactivated.next());
    QCOMPARE(deactivated.value(0).toInt(), 0);

}

QTEST_GUILESS_MAIN(TestUserRepository)
#include "tst_userrepository.moc"
