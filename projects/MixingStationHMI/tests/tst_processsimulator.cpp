
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
    void pump3WithOpenValve3TransfersProduct();
    void closedValve3BlocksProductTransfer();
    void lowerTargetRpmReducesFlow();
    void actualRpmTracksPumpState();
};

void TestProcessSimulator::stoppedPumpDoesNotFillTank()
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
        &pump3,
        &valve3,
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

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
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

    Pump pump3(3);
    Valve valve3(3);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
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

    Pump pump3(3);
    Valve valve3(3);


    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
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

    Pump pump3(3);
    Valve valve3(3);


    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
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

    Pump pump3(3);
    Valve valve3(3);


    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
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


void TestProcessSimulator::pump3WithOpenValve3TransfersProduct()
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
        &pump3,
        &valve3,
        &tank);

    tank.addWater(70.0);
    tank.addConcentrate(30.0);

    QCOMPARE(tank.volume(), 100.0);

    pump3.start();
    valve3.open();

    simulator.simulateStep(10.0);

    QCOMPARE(tank.volume(), 80.0);
}

void TestProcessSimulator::closedValve3BlocksProductTransfer()
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
        &pump3,
        &valve3,
        &tank);

    tank.addWater(70.0);
    tank.addConcentrate(30.0);

    pump3.start();

    // Valve 3 remains closed.
    simulator.simulateStep(10.0);

    QCOMPARE(tank.volume(), 100.0);
}
void TestProcessSimulator::lowerTargetRpmReducesFlow()
{
    Pump pump1(1);
    Valve valve1(1);
    Pump pump2(2);
    Valve valve2(2);
    Pump pump3(3);
    Valve valve3(3);
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1, &valve1,
        &pump2, &valve2,
        &pump3, &valve3,
        &tank);

    pump1.setTargetRpm(750.0);
    valve1.open();
    pump1.start();
    simulator.simulateStep(10.0);
    QCOMPARE(tank.waterVolume(), 5.0);

    pump1.stop();
    valve1.close();

    pump2.setTargetRpm(750.0);
    valve2.open();
    pump2.start();
    simulator.simulateStep(10.0);
    QCOMPARE(tank.concentrateVolume(), 5.0);

    pump2.stop();
    valve2.close();

    pump3.setTargetRpm(750.0);
    valve3.open();
    pump3.start();
    simulator.simulateStep(2.0);
    QCOMPARE(tank.volume(), 8.0);
}

void TestProcessSimulator::actualRpmTracksPumpState()
{
    Pump pump1(1);
    Valve valve1(1);
    Pump pump2(2);
    Valve valve2(2);
    Pump pump3(3);
    Valve valve3(3);
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1, &valve1,
        &pump2, &valve2,
        &pump3, &valve3,
        &tank);

    pump1.setTargetRpm(1000.0);
    pump1.start();

    simulator.simulateStep(1.0);
    QCOMPARE(pump1.actualRpm(), 500.0);

    simulator.simulateStep(1.0);
    QCOMPARE(pump1.actualRpm(), 1000.0);

    pump1.stop();
    simulator.simulateStep(2.0);
    QCOMPARE(pump1.actualRpm(), 0.0);

    pump3.setTargetRpm(750.0);
    pump3.start();
    simulator.simulateStep(2.0);
    QCOMPARE(pump3.actualRpm(), 750.0);
}

QTEST_MAIN(TestProcessSimulator)

#include "tst_processsimulator.moc"