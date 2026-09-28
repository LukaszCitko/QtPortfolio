#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/trendhistorysource.h"

class TestTrendHistorySource : public QObject
{
    Q_OBJECT

private slots:
    void loadsSelectedRun();
};

void TestTrendHistorySource::loadsSelectedRun()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QSqlQuery foreignKeys(database);
    QVERIFY(foreignKeys.exec("PRAGMA foreign_keys = ON"));

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery insertRuns(database);
    QVERIFY(insertRuns.exec(
        "INSERT INTO runs (run_id, kind, started_at_ms) VALUES "
        "('run-1', 'BATCH', 1000), "
        "('run-2', 'DRAIN', 2000)"));

    QSqlQuery insertSamples(database);
    QVERIFY(insertSamples.exec(
        "INSERT INTO trend_samples "
        "(run_id, timestamp_ms, volume_l, temperature_c) VALUES "
        "('run-1', 1200, 20.0, 60.0), "
        "('run-1', 1100, 10.0, 25.0)"));

    TrendHistorySource source(database);

    QVERIFY(source.loadRun("run-1"));
    QCOMPARE(source.sampleCount(), 2);
    QCOMPARE(source.runIdAt(0), QStringLiteral("run-1"));
    QCOMPARE(source.timestampMsAt(0), qint64(1100));
    QCOMPARE(source.volumeAt(0), 10.0);
    QCOMPARE(source.temperatureAt(1), 60.0);

    QVERIFY(source.loadRun("run-2"));
    QCOMPARE(source.runId(), QStringLiteral("run-2"));
    QCOMPARE(source.sampleCount(), 0);

    QVERIFY(source.loadRun(""));
    QCOMPARE(source.runId(), QString());
    QCOMPARE(source.sampleCount(), 0);
}

QTEST_GUILESS_MAIN(TestTrendHistorySource)
#include "tst_trendhistorysource.moc"
