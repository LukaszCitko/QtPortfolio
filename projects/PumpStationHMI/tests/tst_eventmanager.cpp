#include <QtTest>

#include "../src/eventmanager.h"

class TestEventManager : public QObject
{
    Q_OBJECT

private slots:
    void storesEventsInOrder();
    void notifiesViewAboutEachNewEvent();
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

QTEST_GUILESS_MAIN(TestEventManager)

#include "tst_eventmanager.moc"
