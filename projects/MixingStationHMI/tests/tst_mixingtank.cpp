#include <QtTest>

#include "../src/mixingtank.h"

    class TestMixingTank : public QObject
{
    Q_OBJECT

private slots:
    void initialState();
    void waterCanBeAdded();
    void concentrateCanBeAdded();
    void tankCannotBeOverfilled();
    void temperatureCanBeChanged();
    void stateCanBeChanged();
    void productCanBeRemoved();
    void productCannotBeRemovedBelowZero();
    void drainCannotFinishWhileTankContainsProduct();
    void finishedDrainResetsTank();
};

void TestMixingTank::initialState()
{
    MixingTank tank;

    QCOMPARE(tank.capacity(), 120.0);
    QCOMPARE(tank.volume(), 0.0);
    QCOMPARE(tank.levelPercent(), 0.0);
    QCOMPARE(tank.temperature(), 25.0);
    QCOMPARE(tank.waterVolume(), 0.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
    QCOMPARE(tank.state(), MixingTank::State::Empty);
    QVERIFY(!tank.hasFault());
    QVERIFY(!tank.property("fault").toBool());
    QVERIFY(!tank.isActive());
    QVERIFY(!tank.property("active").toBool());
}

void TestMixingTank::waterCanBeAdded()
{
    MixingTank tank;

    tank.addWater(70.0);

    QCOMPARE(tank.volume(), 70.0);
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);

    QCOMPARE(tank.levelPercent(), 70.0);
    QCOMPARE(tank.state(), MixingTank::State::Filling);
}

void TestMixingTank::concentrateCanBeAdded()
{
    MixingTank tank;

    tank.addWater(70.0);
    tank.addConcentrate(30.0);

    QCOMPARE(tank.volume(), 100.0);
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);
    QCOMPARE(tank.levelPercent(), 100.0);
}

void TestMixingTank::tankCannotBeOverfilled()
{
    MixingTank tank;

    tank.addWater(110.0);
    tank.addConcentrate(30.0);

    QCOMPARE(tank.volume(), 120.0);
    QCOMPARE(tank.waterVolume(), 110.0);
    QCOMPARE(tank.concentrateVolume(), 10.0);
}

void TestMixingTank::temperatureCanBeChanged()
{
    MixingTank tank;

    tank.setTemperature(60.0);

    QCOMPARE(tank.temperature(), 60.0);
}

void TestMixingTank::stateCanBeChanged()
{
    MixingTank tank;

    tank.setState(MixingTank::State::ReadyForMixing);

    QCOMPARE(tank.state(), MixingTank::State::ReadyForMixing);
    QCOMPARE(tank.stateText(), QString("READY FOR MIXING"));
    QVERIFY(!tank.hasFault());
    QVERIFY(!tank.isActive());

    tank.setState(MixingTank::State::Mixing);

    QVERIFY(tank.isActive());
    QVERIFY(tank.property("active").toBool());

    tank.setState(MixingTank::State::Fault);

    QCOMPARE(tank.state(), MixingTank::State::Fault);
    QVERIFY(tank.hasFault());
    QVERIFY(tank.property("fault").toBool());
    QVERIFY(!tank.isActive());
}

void TestMixingTank::productCanBeRemoved()
{
    MixingTank tank;

    tank.addWater(70.0);
    tank.addConcentrate(30.0);

    QCOMPARE(tank.volume(), 100.0);
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);

    tank.removeProduct(20.0);

    QCOMPARE(tank.volume(), 80.0);

    // Recipe composition remains unchanged.
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);
}

void TestMixingTank::productCannotBeRemovedBelowZero()
{
    MixingTank tank;

    tank.addWater(70.0);
    tank.addConcentrate(30.0);

    tank.removeProduct(150.0);

    QCOMPARE(tank.volume(), 0.0);

    // Recipe composition remains unchanged.
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);
}

void TestMixingTank::drainCannotFinishWhileTankContainsProduct()
{
    MixingTank tank;
    tank.addWater(10.0);
    tank.setState(MixingTank::State::Draining);

    QVERIFY(!tank.resetAfterDrain());
    QCOMPARE(tank.volume(), 10.0);
    QCOMPARE(tank.waterVolume(), 10.0);
    QCOMPARE(tank.state(), MixingTank::State::Draining);
}

void TestMixingTank::finishedDrainResetsTank()
{
    MixingTank tank;
    tank.addWater(20.0);
    tank.addConcentrate(10.0);
    tank.setTemperature(60.0);
    tank.setState(MixingTank::State::Draining);

    tank.removeProduct(30.0);

    QVERIFY(tank.resetAfterDrain());
    QCOMPARE(tank.volume(), 0.0);
    QCOMPARE(tank.waterVolume(), 0.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
    QCOMPARE(tank.temperature(), 25.0);
    QCOMPARE(tank.state(), MixingTank::State::Empty);
}
QTEST_GUILESS_MAIN(TestMixingTank)

#include "tst_mixingtank.moc"
