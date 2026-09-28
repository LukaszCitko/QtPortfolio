#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/operatorsession.h"

class TestOperatorSession : public QObject
{
    Q_OBJECT

private slots:
    void selectsOnlyActiveUserFromDatabase();
};

void TestOperatorSession::selectsOnlyActiveUserFromDatabase()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery insertUsers(database);
    QVERIFY(insertUsers.exec(
        "INSERT INTO users (user_id, display_name, role, is_active) VALUES "
        "('tech-1', 'Test Technician', 'TECHNICIAN', 1), "
        "('admin-1', 'Inactive Admin', 'ADMIN', 0)"));

    OperatorSession session(database);

    QVERIFY(!session.selected());
    QVERIFY(session.selectOperator("tech-1"));
    QVERIFY(session.selected());
    QCOMPARE(session.operatorId(), QStringLiteral("tech-1"));
    QCOMPARE(session.operatorName(), QStringLiteral("Test Technician"));
    QCOMPARE(session.operatorRole(), QStringLiteral("TECHNICIAN"));
    QVERIFY(session.canControlBatch());
    QVERIFY(session.canResetFault());
    QVERIFY(!session.canManageUsers());
    QVERIFY(session.canApproveDrain());
    QVERIFY(!session.selectOperator("admin-1"));
    QCOMPARE(session.operatorId(), QStringLiteral("tech-1"));

    QSqlQuery updateRole(database);
    QVERIFY(updateRole.exec(
        "UPDATE users SET role = 'ADMIN' WHERE user_id = 'tech-1'"));

    QVERIFY(session.selectOperator("tech-1"));
    QCOMPARE(session.operatorRole(), QStringLiteral("ADMIN"));
    QVERIFY(session.canApproveDrain());
    QVERIFY(session.canControlBatch());
    QVERIFY(session.canResetFault());
    QVERIFY(session.canManageUsers());
}

QTEST_GUILESS_MAIN(TestOperatorSession)

#include "tst_operatorsession.moc"
