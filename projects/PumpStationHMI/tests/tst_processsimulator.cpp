
#include <QtTest>

#include "../src/mixingtank.h"
#include "../src/processsimulator.h"
#include "../src/pump.h"
#include "../src/valve.h"

    class TestProcessSimulator : public QObject
{
    Q_OBJECT

private slots:
    void stoppedPumpDoesNotFillTank();
    void closedValveBlocksWaterFlow();
    void runningPumpWithOpenValveFillsTank();
    void runningPump2WithOpenValve2FillsTankWithConcentrate();
    void waterAndConcentrateCanBeAdded();
    void waterFillingCanBeStoppedBeforeConcentrateDosing();

};

void TestProcessSimulator::stoppedPumpDoesNotFillTank()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    valve1.open();

    QTest::qWait(500);

    QCOMPARE(tank.volume(), 0.0);
}

void TestProcessSimulator::closedValveBlocksWaterFlow()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    pump1.start();

    QTest::qWait(500);

    QCOMPARE(tank.volume(), 0.0);
}

void TestProcessSimulator::runningPumpWithOpenValveFillsTank()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    valve1.open();
    pump1.start();

    QTest::qWait(500);

    QVERIFY(tank.volume() > 0.0);
}

    void TestProcessSimulator::runningPump2WithOpenValve2FillsTankWithConcentrate()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    valve2.open();
    pump2.start();

    QTest::qWait(500);

    QVERIFY(tank.concentrateVolume() > 0.0);
    QCOMPARE(tank.waterVolume(), 0.0);
}

    void TestProcessSimulator::waterAndConcentrateCanBeAdded()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);


    // 70 L water at 1 L/s.
//QTest::qWait(7000);


    valve1.open();
    pump1.start();
    simulator.simulateStep(70.0);

    QCOMPARE(tank.waterVolume(), 70.0);

    pump1.stop();
    valve1.close();
    // 30 L concentrate at 1 L/s.
// QTest::qWait(3000);

    valve2.open();
    pump2.start();
    simulator.simulateStep(30.0);

    QCOMPARE(tank.concentrateVolume(), 30.0);
}


    void TestProcessSimulator::waterFillingCanBeStoppedBeforeConcentrateDosing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &tank);

    // Fill the tank with 70 L of water.
    valve1.open();
    pump1.start();

    simulator.simulateStep(70.0);

    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
    QCOMPARE(tank.volume(), 70.0);

    // Stop water flow.
    pump1.stop();
    valve1.close();

    // Start concentrate dosing.
    valve2.open();
    pump2.start();

    simulator.simulateStep(30.0);

    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);
    QCOMPARE(tank.volume(), 100.0);
}

QTEST_MAIN(TestProcessSimulator)

#include "tst_processsimulator.moc"