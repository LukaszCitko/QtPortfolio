#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/demouserseeder.h"

class TestDemoUserSeeder : public QObject
{
    Q_OBJECT

private slots:
    void seedsOnlyEmptyDatabase();
};

void TestDemoUserSeeder::seedsOnlyEmptyDatabase()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery custom(database);
    QVERIFY(custom.exec(
        "INSERT INTO users (user_id, display_name, role) "
        "VALUES ('existing-user', 'Existing User', 'OPERATOR')"));

    QVERIFY2(seedDemoUsersIfEmpty(database, &error), qPrintable(error));

    QSqlQuery existingCount(database);
    QVERIFY(existingCount.exec("SELECT COUNT(*) FROM users"));
    QVERIFY(existingCount.next());
    QCOMPARE(existingCount.value(0).toInt(), 1);

    QSqlQuery clear(database);
    QVERIFY(clear.exec("DELETE FROM users"));

    QVERIFY2(seedDemoUsersIfEmpty(database, &error), qPrintable(error));
    QVERIFY2(seedDemoUsersIfEmpty(database, &error), qPrintable(error));

    QSqlQuery demoCount(database);
    QVERIFY(demoCount.exec("SELECT COUNT(*) FROM users"));
    QVERIFY(demoCount.next());
    QCOMPARE(demoCount.value(0).toInt(), 4);

    QSqlQuery roles(database);
    QVERIFY(roles.exec(
        "SELECT role, COUNT(*) FROM users GROUP BY role"));

    int operators = 0;
    int technicians = 0;
    int admins = 0;

    while (roles.next()) {
        const QString role = roles.value(0).toString();
        const int count = roles.value(1).toInt();

        if (role == "OPERATOR")
            operators = count;
        else if (role == "TECHNICIAN")
            technicians = count;
        else if (role == "ADMIN")
            admins = count;
    }

    QCOMPARE(operators, 2);
    QCOMPARE(technicians, 1);
    QCOMPARE(admins, 1);
}

QTEST_GUILESS_MAIN(TestDemoUserSeeder)
#include "tst_demouserseeder.moc"
