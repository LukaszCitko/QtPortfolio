#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>

#include "../src/database/databaseschema.h"
#include "../src/database/eventrepository.h"
#include "../src/database/persistenceretryqueue.h"
#include "../src/database/runrepository.h"
#include "../src/database/trendsamplerepository.h"

class TestPersistenceRetryQueue : public QObject
{
    Q_OBJECT

private slots:
    void retriesRunStartAfterDatabaseReopens();
    void retriesEventAfterDatabaseReopens();
    void retriesRunBeforeItsSample();
    void retriesRunStartBeforeFinish();
};

void TestPersistenceRetryQueue::retriesRunStartAfterDatabaseReopens()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(directory.filePath("history.sqlite"));
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    RunRepository runs(database);
    EventRepository events(database);
    TrendSampleRepository samples(database);
    PersistenceRetryQueue queue(runs, events, samples);

    database.close();
    queue.submitRunStart(PersistenceRetryQueue::RunStartKind::ObservedBatch, "observed-1", "", 1234);
    queue.submitRunStart(PersistenceRetryQueue::RunStartKind::ObservedBatch, "observed-1", "", 2222);
    QVERIFY(database.open());

    const auto runExists = [&database]() {
        QSqlQuery query(database);
        return query.exec("SELECT 1 FROM runs WHERE run_id = 'observed-1'") && query.next();
    };
    QTRY_VERIFY_WITH_TIMEOUT(runExists(), 5000);

    QSqlQuery savedRun(database);
    QVERIFY(savedRun.exec("SELECT kind, batch_operator_user_id, started_at_ms FROM runs WHERE run_id = 'observed-1'"));
    QVERIFY(savedRun.next());
    QCOMPARE(savedRun.value(0).toString(), QStringLiteral("BATCH"));
    QVERIFY(savedRun.value(1).isNull());
    QCOMPARE(savedRun.value(2).toLongLong(), qint64(1234));
}

void TestPersistenceRetryQueue::retriesEventAfterDatabaseReopens()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "event-retry-test");
    database.setDatabaseName(directory.filePath("events.sqlite"));
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    RunRepository runs(database);
    EventRepository events(database);
    TrendSampleRepository samples(database);
    PersistenceRetryQueue queue(runs, events, samples);
    QSignalSpy savedSpy(&queue, &PersistenceRetryQueue::eventSaved);

    database.close();
    queue.submitEvent("", 1234, "INFO", "TEST", "Queued event");
    QVERIFY(queue.hasPendingWrites());
    QCOMPARE(savedSpy.count(), 0);

    QVERIFY(database.open());
    QTRY_COMPARE_WITH_TIMEOUT(savedSpy.count(), 1, 5000);
    QVERIFY(!queue.hasPendingWrites());
    QSqlQuery savedEvent(database);
    QVERIFY(savedEvent.exec("SELECT run_id, timestamp_ms, level, source, message FROM events"));
    QVERIFY(savedEvent.next());
    QVERIFY(savedEvent.value(0).isNull());
    QCOMPARE(savedEvent.value(1).toLongLong(), qint64(1234));
    QCOMPARE(savedEvent.value(2).toString(), QStringLiteral("INFO"));
    QCOMPARE(savedEvent.value(3).toString(), QStringLiteral("TEST"));
    QCOMPARE(savedEvent.value(4).toString(), QStringLiteral("Queued event"));
    QVERIFY(!savedEvent.next());
}

void TestPersistenceRetryQueue::retriesRunBeforeItsSample()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "sample-retry-test");
    database.setDatabaseName(directory.filePath("samples.sqlite"));
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    RunRepository runs(database);
    EventRepository events(database);
    TrendSampleRepository samples(database);
    PersistenceRetryQueue queue(runs, events, samples);

    database.close();
    queue.submitRunStart(PersistenceRetryQueue::RunStartKind::ObservedBatch, "observed-2", "", 1000);
    queue.submitSample("observed-2", 1500, 2.5, 21.0);

    QVERIFY(database.open());
    QSqlQuery foreignKeys(database);
    QVERIFY(foreignKeys.exec("PRAGMA foreign_keys = ON"));

    const auto sampleExists = [&database]() {
        QSqlQuery query(database);
        return query.exec("SELECT 1 FROM trend_samples WHERE run_id = 'observed-2'") && query.next();
    };
    QTRY_VERIFY_WITH_TIMEOUT(sampleExists(), 5000);

    QSqlQuery savedSample(database);
    QVERIFY(savedSample.exec("SELECT timestamp_ms, volume_l, temperature_c FROM trend_samples WHERE run_id = 'observed-2'"));
    QVERIFY(savedSample.next());
    QCOMPARE(savedSample.value(0).toLongLong(), qint64(1500));
    QCOMPARE(savedSample.value(1).toDouble(), 2.5);
    QCOMPARE(savedSample.value(2).toDouble(), 21.0);
    QVERIFY(!savedSample.next());
}

void TestPersistenceRetryQueue::retriesRunStartBeforeFinish()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "finish-retry-test");
    database.setDatabaseName(directory.filePath("runs.sqlite"));
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    RunRepository runs(database);
    EventRepository events(database);
    TrendSampleRepository samples(database);
    PersistenceRetryQueue queue(runs, events, samples);

    database.close();
    queue.submitRunStart(PersistenceRetryQueue::RunStartKind::ObservedBatch, "observed-3", "", 1000);
    queue.submitRunFinish("observed-3", "COMPLETED", 2000);
    queue.submitRunFinish("observed-3", "COMPLETED", 3000);
    QVERIFY(database.open());

    const auto runFinished = [&database]() {
        QSqlQuery query(database);
        return query.exec("SELECT 1 FROM runs WHERE run_id = 'observed-3' AND outcome = 'COMPLETED'") && query.next();
    };
    QTRY_VERIFY_WITH_TIMEOUT(runFinished(), 5000);

    QSqlQuery savedRun(database);
    QVERIFY(savedRun.exec("SELECT started_at_ms, ended_at_ms, outcome FROM runs WHERE run_id = 'observed-3'"));
    QVERIFY(savedRun.next());
    QCOMPARE(savedRun.value(0).toLongLong(), qint64(1000));
    QCOMPARE(savedRun.value(1).toLongLong(), qint64(2000));
    QCOMPARE(savedRun.value(2).toString(), QStringLiteral("COMPLETED"));
    QVERIFY(!savedRun.next());
}


QTEST_GUILESS_MAIN(TestPersistenceRetryQueue)
#include "tst_persistenceretryqueue.moc"