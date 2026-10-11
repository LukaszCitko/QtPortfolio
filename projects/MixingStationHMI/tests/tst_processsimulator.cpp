
#include <QtTest>

#include "../src/mixingtank.h"
#include "processconfiguration.h"
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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

    pump1.setTargetRpm(ProcessConfiguration::Pump1DefaultRpm);
    valve1.open();
    pump1.start();

    const double waterTargetLitres = static_cast<double>(ProcessConfiguration::WaterTargetDecilitres) / 10.0;
    const double waterMaximumFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::WaterFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
    const double waterFlowLitresPerSecond = waterMaximumFlowLitresPerSecond * pump1.targetRpm() / Pump::MaxRpm;
    const double waterFillingSeconds = waterTargetLitres / waterFlowLitresPerSecond;

    simulator.simulateStep(waterFillingSeconds);

    QCOMPARE(tank.waterVolume(), waterTargetLitres);

    pump1.stop();
    valve1.close();

    pump2.setTargetRpm(ProcessConfiguration::Pump2DefaultRpm);
    valve2.open();
    pump2.start();

    const double concentrateTargetLitres = static_cast<double>(ProcessConfiguration::ConcentrateTargetDecilitres) / 10.0;
    const double concentrateMaximumFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::ConcentrateFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
    const double concentrateFlowLitresPerSecond = concentrateMaximumFlowLitresPerSecond * pump2.targetRpm() / Pump::MaxRpm;
    const double concentrateFillingSeconds = concentrateTargetLitres / concentrateFlowLitresPerSecond;

    simulator.simulateStep(concentrateFillingSeconds);

    QCOMPARE(tank.concentrateVolume(), concentrateTargetLitres);
    QCOMPARE(tank.volume(), waterTargetLitres + concentrateTargetLitres);
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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

    pump1.setTargetRpm(ProcessConfiguration::Pump1DefaultRpm);
    valve1.open();
    pump1.start();

    const double waterTargetLitres = static_cast<double>(ProcessConfiguration::WaterTargetDecilitres) / 10.0;
    const double waterMaximumFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::WaterFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
    const double waterFlowLitresPerSecond = waterMaximumFlowLitresPerSecond * pump1.targetRpm() / Pump::MaxRpm;
    const double waterFillingSeconds = waterTargetLitres / waterFlowLitresPerSecond;

    simulator.simulateStep(waterFillingSeconds);

    QCOMPARE(tank.waterVolume(), waterTargetLitres);
    QCOMPARE(tank.concentrateVolume(), 0.0);
    QCOMPARE(tank.volume(), waterTargetLitres);

    pump1.stop();
    valve1.close();

    pump2.setTargetRpm(ProcessConfiguration::Pump2DefaultRpm);
    valve2.open();
    pump2.start();

    const double concentrateTargetLitres = static_cast<double>(ProcessConfiguration::ConcentrateTargetDecilitres) / 10.0;
    const double concentrateMaximumFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::ConcentrateFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
    const double concentrateFlowLitresPerSecond = concentrateMaximumFlowLitresPerSecond * pump2.targetRpm() / Pump::MaxRpm;
    const double concentrateFillingSeconds = concentrateTargetLitres / concentrateFlowLitresPerSecond;

    simulator.simulateStep(concentrateFillingSeconds);

    QCOMPARE(tank.waterVolume(), waterTargetLitres);
    QCOMPARE(tank.concentrateVolume(), concentrateTargetLitres);
    QCOMPARE(tank.volume(), waterTargetLitres + concentrateTargetLitres);
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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

    const double mixtureTargetLitres = static_cast<double>(ProcessConfiguration::MixtureTargetDecilitres) / 10.0;

    tank.addWater(static_cast<double>(ProcessConfiguration::WaterTargetDecilitres) / 10.0);
    tank.addConcentrate(static_cast<double>(ProcessConfiguration::ConcentrateTargetDecilitres) / 10.0);

    QCOMPARE(tank.volume(), mixtureTargetLitres);

    pump3.setTargetRpm(ProcessConfiguration::Pump3DefaultRpm);
    pump3.start();
    valve3.open();

    const double productMaximumFlowLitresPerSecond = static_cast<double>(ProcessConfiguration::ProductFlowAtMaximumRpmDecilitresPerSecond) / 10.0;
    const double productFlowLitresPerSecond = productMaximumFlowLitresPerSecond * pump3.targetRpm() / Pump::MaxRpm;
    const double transferSeconds = mixtureTargetLitres / productFlowLitresPerSecond;

    QVERIFY(transferSeconds <= 10.0); // Ten seconds is acceptable for the demo version.

    simulator.simulateStep(transferSeconds);

    QCOMPARE(tank.volume(), 0.0);
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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);
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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

    pump1.setTargetRpm(500.0);
    valve1.open();
    pump1.start();

    simulator.simulateStep(0.5);

    const double volumeAtHalfRpm = tank.waterVolume();

    QCOMPARE(volumeAtHalfRpm, 5.0);

    pump1.setTargetRpm(Pump::MaxRpm);

    simulator.simulateStep(0.5);

    const double volumeAtMaximumRpm = tank.waterVolume() - volumeAtHalfRpm;

    QCOMPARE(volumeAtMaximumRpm, 10.0);
    QCOMPARE(volumeAtMaximumRpm, volumeAtHalfRpm * 2.0);
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

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &tank);

    pump1.setTargetRpm(Pump::MaxRpm);
    pump1.start();

    const double halfRampSeconds = (Pump::MaxRpm / 2.0) / static_cast<double>(ProcessConfiguration::PumpRpmChangePerSecond);

    simulator.simulateStep(halfRampSeconds);

    QCOMPARE(pump1.actualRpm(), Pump::MaxRpm / 2.0);

    simulator.simulateStep(halfRampSeconds);

    QCOMPARE(pump1.actualRpm(), Pump::MaxRpm);

    pump1.stop();

    const double fullRampSeconds = Pump::MaxRpm / static_cast<double>(ProcessConfiguration::PumpRpmChangePerSecond);

    simulator.simulateStep(fullRampSeconds);

    QCOMPARE(pump1.actualRpm(), 0.0);

    pump3.setTargetRpm(750.0);
    pump3.start();

    const double pump3RampSeconds = pump3.targetRpm() / static_cast<double>(ProcessConfiguration::PumpRpmChangePerSecond);

    simulator.simulateStep(pump3RampSeconds);

    QCOMPARE(pump3.actualRpm(), 750.0);
}

QTEST_GUILESS_MAIN(TestProcessSimulator)

#include "tst_processsimulator.moc"
