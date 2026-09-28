#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/database/eventrepository.h"

class TestEventRepository : public QObject
{
    Q_OBJECT

private slots:
    void storesBatchAndStandaloneEvents();
};

void TestEventRepository::storesBatchAndStandaloneEvents()
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

    EventRepository repository(database);

    QVERIFY2(repository.saveEvent("batch-1", 1100, "INFO",
                                  "BATCH", "Batch started", &error),
             qPrintable(error));

    QVERIFY2(repository.saveEvent("", 1200, "WARNING",
                                  "TK1", "Temperature high", &error),
             qPrintable(error));

    QSqlQuery savedEvents(database);
    QVERIFY(savedEvents.exec(
        "SELECT run_id, timestamp_ms, level, source, message "
        "FROM events ORDER BY event_id"));

    QVERIFY(savedEvents.next());
    QCOMPARE(savedEvents.value(0).toString(), QStringLiteral("batch-1"));
    QCOMPARE(savedEvents.value(1).toLongLong(), qint64(1100));
    QCOMPARE(savedEvents.value(2).toString(), QStringLiteral("INFO"));
    QCOMPARE(savedEvents.value(3).toString(), QStringLiteral("BATCH"));
    QCOMPARE(savedEvents.value(4).toString(), QStringLiteral("Batch started"));

    QVERIFY(savedEvents.next());
    QVERIFY(savedEvents.value(0).isNull());
    QCOMPARE(savedEvents.value(1).toLongLong(), qint64(1200));
    QCOMPARE(savedEvents.value(2).toString(), QStringLiteral("WARNING"));

    QVERIFY(!savedEvents.next());

    QVERIFY(!repository.saveEvent("missing-run", 1300, "INFO",
                                  "BATCH", "Invalid event", &error));
}

QTEST_GUILESS_MAIN(TestEventRepository)
#include "tst_eventrepository.moc"
