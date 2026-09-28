#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/runrepository.h"

class TestRunRepository : public QObject
{
    Q_OBJECT

private slots:
    void storesBatchLifecycle();
};

void TestRunRepository::storesBatchLifecycle()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QSqlQuery foreignKeys(database);
    QVERIFY(foreignKeys.exec("PRAGMA foreign_keys = ON"));

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery insertUser(database);
    QVERIFY(insertUser.exec(
        "INSERT INTO users (user_id, display_name, role) "
        "VALUES ('operator-1', 'Test Operator', 'OPERATOR')"));

    RunRepository repository(database);

    QVERIFY2(repository.beginBatch("batch-1", "operator-1", 1000, &error), qPrintable(error));
    QVERIFY2(repository.finishRun("batch-1", "COMPLETED", 2000, &error), qPrintable(error));

    QSqlQuery savedRun(database);
    QVERIFY(savedRun.exec(
        "SELECT kind, batch_operator_user_id, started_at_ms, "
        "ended_at_ms, outcome FROM runs WHERE run_id = 'batch-1'"));
    QVERIFY(savedRun.next());

    QCOMPARE(savedRun.value(0).toString(), QStringLiteral("BATCH"));
    QCOMPARE(savedRun.value(1).toString(), QStringLiteral("operator-1"));
    QCOMPARE(savedRun.value(2).toLongLong(), qint64(1000));
    QCOMPARE(savedRun.value(3).toLongLong(), qint64(2000));
    QCOMPARE(savedRun.value(4).toString(), QStringLiteral("COMPLETED"));

    QVERIFY(!repository.beginBatch("batch-2", "missing-user", 3000, &error));
    QVERIFY(!repository.finishRun("batch-1", "COMPLETED", 4000, &error));
    QVERIFY2(repository.beginDrain("drain-1", 5000, &error), qPrintable(error));
    QVERIFY2(repository.finishRun("drain-1", "DRAINED", 6000, &error), qPrintable(error));

    QSqlQuery savedDrain(database);
    QVERIFY(savedDrain.exec(
        "SELECT kind, batch_operator_user_id, started_at_ms, "
        "ended_at_ms, outcome FROM runs WHERE run_id = 'drain-1'"));
    QVERIFY(savedDrain.next());

    QCOMPARE(savedDrain.value(0).toString(), QStringLiteral("DRAIN"));
    QVERIFY(savedDrain.value(1).isNull());
    QCOMPARE(savedDrain.value(2).toLongLong(), qint64(5000));
    QCOMPARE(savedDrain.value(3).toLongLong(), qint64(6000));
    QCOMPARE(savedDrain.value(4).toString(), QStringLiteral("DRAINED"));
}

QTEST_GUILESS_MAIN(TestRunRepository)
#include "tst_runrepository.moc"
