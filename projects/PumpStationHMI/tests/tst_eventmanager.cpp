#include <QtTest>

#include "../src/eventmanager.h"

class TestEventManager : public QObject
{
    Q_OBJECT

private slots:
    void storesEventsInOrder();
    void notifiesViewAboutEachNewEvent();
    void storesBatchIdWithEachEvent();
    void emitsDatabaseReadyEvent();
};

void TestEventManager::storesEventsInOrder()
{
    EventManager manager;

    QCOMPARE(manager.eventCount(), 0);

    manager.addInfo("BATCH", "Batch started");
    manager.addAlarm("P1", "Device fault detected");
    manager.addInfo("P1", "Device fault cleared");

    QCOMPARE(manager.eventCount(), 3);

    QCOMPARE(manager.eventLevel(0), QString("INFO"));
    QCOMPARE(manager.eventSource(0), QString("BATCH"));
    QCOMPARE(manager.eventMessage(0), QString("Batch started"));

    QCOMPARE(manager.eventLevel(1), QString("ALARM"));
    QCOMPARE(manager.eventSource(1), QString("P1"));
    QCOMPARE(manager.eventMessage(1), QString("Device fault detected"));

    QCOMPARE(manager.eventLevel(2), QString("INFO"));
    QCOMPARE(manager.eventMessage(2), QString("Device fault cleared"));

    QVERIFY(!manager.eventTime(0).isEmpty());
    QVERIFY(!manager.eventTime(1).isEmpty());
    QVERIFY(!manager.eventTime(2).isEmpty());
}

void TestEventManager::notifiesViewAboutEachNewEvent()
{
    EventManager manager;
    QSignalSpy addedSpy(&manager, &EventManager::eventAdded);
    QSignalSpy currentSpy(&manager, &EventManager::currentEventChanged);

    manager.addWarning("BATCH", "Batch paused");

    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(currentSpy.count(), 1);
    QCOMPARE(manager.eventCount(), 1);
    QCOMPARE(manager.currentLevel(), EventManager::Level::Warning);
    QCOMPARE(manager.currentMessage(), QString("Batch paused"));

    manager.addInfo("BATCH", "Batch resumed");

    QCOMPARE(addedSpy.count(), 2);
    QCOMPARE(currentSpy.count(), 2);
    QCOMPARE(manager.eventCount(), 2);
    QCOMPARE(manager.currentMessage(), QString("Batch resumed"));
}


void TestEventManager::storesBatchIdWithEachEvent()
{
    EventManager manager;

    manager.addInfo("SYSTEM", "Ready");

    manager.setBatchId("batch-123");
    manager.addInfo("BATCH", "Batch started");
    manager.addAlarm("M1", "Device fault detected");

    manager.setBatchId(QString());
    manager.addInfo("SYSTEM", "Ready for next batch");

    QCOMPARE(manager.eventBatchId(0), QString());
    QCOMPARE(manager.eventBatchId(1), QString("batch-123"));
    QCOMPARE(manager.eventBatchId(2), QString("batch-123"));
    QCOMPARE(manager.eventBatchId(3), QString());
    QCOMPARE(manager.eventBatchId(-1), QString());
}

void TestEventManager::emitsDatabaseReadyEvent()
{
    EventManager manager;
    QSignalSpy recordedSpy(&manager, &EventManager::eventRecorded);

    manager.setBatchId("batch-123");
    manager.addWarning("TK1", "Temperature reached 90 °C");

    QCOMPARE(recordedSpy.count(), 1);

    const QList<QVariant> arguments = recordedSpy.takeFirst();

    QCOMPARE(arguments.at(0).toString(), QString("batch-123"));
    QVERIFY(arguments.at(1).toLongLong() > 0);
    QCOMPARE(arguments.at(2).toString(), QString("WARNING"));
    QCOMPARE(arguments.at(3).toString(), QString("TK1"));
    QCOMPARE(arguments.at(4).toString(), QString("Temperature reached 90 °C"));
}

QTEST_GUILESS_MAIN(TestEventManager)

#include "tst_eventmanager.moc"
