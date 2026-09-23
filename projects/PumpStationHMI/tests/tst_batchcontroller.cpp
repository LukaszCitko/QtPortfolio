
#include <QtTest>

#include "../src/batchcontroller.h"
#include "../src/mixingtank.h"
#include "../src/pump.h"
#include "../src/valve.h"
#include "../src/processsimulator.h"

class TestBatchController : public QObject
{
    Q_OBJECT

private slots:
    void startBatchStartsWaterFilling();
    void waterFillingAutomaticallyChangesToConcentrateDosing();
    void concentrateDosingAutomaticallyChangesToTemperatureCheck();
    void correctTemperatureStartsMixing();
    void incorrectTemperatureKeepsTemperatureCheck();
    void mixingFinishesAfterSixtySeconds();
    void readyForTransferStartsTransferring();
};

void TestBatchController::startBatchStartsWaterFilling()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    QCOMPARE(controller.stateText(), QString("IDLE"));

    controller.startBatch();

    QCOMPARE(
        controller.stateText(),
        QString("FILLING WATER"));

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
}

void TestBatchController::waterFillingAutomaticallyChangesToConcentrateDosing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    controller.startBatch();

    QCOMPARE(
        controller.stateText(),
        QString("FILLING WATER"));

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    simulator.simulateStep(70.0);

    QCOMPARE(
        controller.stateText(),
        QString("DOSING CONCENTRATE"));

    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());

    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());

    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
}
void TestBatchController::concentrateDosingAutomaticallyChangesToTemperatureCheck() {

    Pump pump1(1);
    Valve valve1(1);
    Pump pump2(2);
    Valve valve2(2);
    Pump pump3(3);
    Valve valve3(3);
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    controller.startBatch();
    simulator.simulateStep(70.0);
    QCOMPARE( controller.stateText(), QString("DOSING CONCENTRATE"));
    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());
    simulator.simulateStep(30.0);
    QCOMPARE( controller.stateText(), QString("TEMPERATURE CHECK"));
    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);
    QCOMPARE(tank.volume(), 100.0);
}

void TestBatchController::correctTemperatureStartsMixing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    QCOMPARE(tank.volume(), 100.0);

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));
}

void TestBatchController::incorrectTemperatureKeepsTemperatureCheck()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(55.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(65.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));
}


void TestBatchController::mixingFinishesAfterSixtySeconds()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(29.9);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(0.1);

    QCOMPARE(
        controller.stateText(),
        QString("READY FOR TRANSFER"));
}

void TestBatchController::readyForTransferStartsTransferring()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    controller.simulateStep(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("READY FOR TRANSFER"));

    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());

    controller.startTransfer();

    QCOMPARE(
        controller.stateText(),
        QString("TRANSFERRING"));

    QVERIFY(pump3.isRunning());
    QVERIFY(valve3.isOpen());
}

QTEST_MAIN(TestBatchController)

#include "tst_batchcontroller.moc"
