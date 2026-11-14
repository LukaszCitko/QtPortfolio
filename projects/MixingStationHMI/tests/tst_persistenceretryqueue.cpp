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
    void reportsConflictingCompletions();
    void reportsEventOverflow();
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

void TestPersistenceRetryQueue::reportsConflictingCompletions()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "completion-conflict-test");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    RunRepository runs(database);
    EventRepository events(database);
    TrendSampleRepository samples(database);

    QVERIFY2(runs.beginObservedBatch("stored-run", 1000, &error), qPrintable(error));
    QVERIFY2(runs.finishRun("stored-run", "COMPLETED", 2000, &error), qPrintable(error));

    PersistenceRetryQueue storedConflict(runs, events, samples);
    storedConflict.submitRunFinish("stored-run", "INTERRUPTED", 3000);
    QVERIFY(storedConflict.hasPermanentFailure());
    QVERIFY(!storedConflict.hasPendingWrites());

    database.close();

    PersistenceRetryQueue pendingConflict(runs, events, samples);
    pendingConflict.submitRunFinish("pending-run", "COMPLETED", 4000);
    QVERIFY(pendingConflict.hasPendingWrites());
    QVERIFY(!pendingConflict.hasPermanentFailure());

    pendingConflict.submitRunFinish("pending-run", "INTERRUPTED", 5000);
    QVERIFY(pendingConflict.hasPendingWrites());
    QVERIFY(pendingConflict.hasPermanentFailure());
}

void TestPersistenceRetryQueue::reportsEventOverflow()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "event-overflow-test");
    database.setDatabaseName(directory.filePath("overflow.sqlite"));
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    RunRepository runs(database);
    EventRepository events(database);
    TrendSampleRepository samples(database);
    PersistenceRetryQueue queue(runs, events, samples);

    database.close();

    for (int i = 0; i < 1001; ++i)
        queue.submitEvent("", i, "INFO", "TEST", "Event");

    QVERIFY(queue.hasPendingWrites());
    QVERIFY(queue.hasPermanentFailure());

    QVERIFY(database.open());
    QTRY_VERIFY_WITH_TIMEOUT(!queue.hasPendingWrites(), 20000);

    QSqlQuery savedEvents(database);
    QVERIFY(savedEvents.exec("SELECT COUNT(*), MIN(timestamp_ms), MAX(timestamp_ms) FROM events"));
    QVERIFY(savedEvents.next());
    QCOMPARE(savedEvents.value(0).toInt(), 1000);
    QCOMPARE(savedEvents.value(1).toInt(), 1);
    QCOMPARE(savedEvents.value(2).toInt(), 1000);
    QVERIFY(queue.hasPermanentFailure());
}

QTEST_GUILESS_MAIN(TestPersistenceRetryQueue)
#include "tst_persistenceretryqueue.moc"

