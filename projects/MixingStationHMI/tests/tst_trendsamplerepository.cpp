#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/trendsamplerepository.h"

class TestTrendSampleRepository : public QObject
{
    Q_OBJECT

private slots:
    void storesSampleForExistingRun();
};

void TestTrendSampleRepository::storesSampleForExistingRun()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QSqlQuery foreignKeys(database);
    QVERIFY(foreignKeys.exec("PRAGMA foreign_keys = ON"));

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery insertRun(database);
    QVERIFY(insertRun.exec(
        "INSERT INTO runs (run_id, kind, started_at_ms) "
        "VALUES ('batch-1', 'BATCH', 1000)"));

    TrendSampleRepository repository(database);

    QVERIFY2(repository.saveSample("batch-1", 1100, 35.5, 60.0, &error),
             qPrintable(error));

    QSqlQuery savedSample(database);
    QVERIFY(savedSample.exec(
        "SELECT run_id, timestamp_ms, volume_l, temperature_c "
        "FROM trend_samples"));

    QVERIFY(savedSample.next());
    QCOMPARE(savedSample.value(0).toString(), QStringLiteral("batch-1"));
    QCOMPARE(savedSample.value(1).toLongLong(), qint64(1100));
    QCOMPARE(savedSample.value(2).toDouble(), 35.5);
    QCOMPARE(savedSample.value(3).toDouble(), 60.0);
    QVERIFY(!savedSample.next());

    QVERIFY(!repository.saveSample("", 1200, 40.0, 60.0, &error));
    QVERIFY(!repository.saveSample("missing-run", 1300, 45.0, 60.0, &error));
}

QTEST_GUILESS_MAIN(TestTrendSampleRepository)
#include "tst_trendsamplerepository.moc"
