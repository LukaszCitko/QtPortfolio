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
}

void TestMixingTank::waterCanBeAdded()
{
    MixingTank tank;

    tank.addWater(70.0);

    QCOMPARE(tank.volume(), 70.0);
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
    QCOMPARE(tank.levelPercent(), 70.0 / 120.0 * 100.0);
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
    QCOMPARE(tank.levelPercent(), 100.0 / 120.0 * 100.0);
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

QTEST_MAIN(TestMixingTank)

#include "tst_mixingtank.moc"