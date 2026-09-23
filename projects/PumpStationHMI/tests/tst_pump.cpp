#include<QtTest>
#include "../src/pump.h"
class TestPump : public QObject
{
    Q_OBJECT
private slots:
    void initialState();
    void startPump();
    void stopPump();
    void faultState();
    void resetFault();
    void stopDoesNotClearFault();
    void rpmValues();
    void temperature();
};

void TestPump::initialState()
{
    Pump pump(1);

    QCOMPARE(pump.state(), Pump::State::Stopped);

    QVERIFY(!pump.isRunning());
}

void TestPump::temperature()
{
    Pump pump(1);

    QCOMPARE(pump.temperatureFromSensor(), 25.0);

    pump.setTemperatureFromSensor(40.0);

    QCOMPARE(pump.temperatureFromSensor(), 40.0);
}

void TestPump::startPump()
{
    Pump pump(1);

    pump.start();

    QVERIFY(pump.isRunning());
}
void TestPump::stopPump()
{
    Pump pump(1);

    pump.start();
    pump.stop();

    QVERIFY(!pump.isRunning());
}

void TestPump::faultState()
{
    Pump pump(1);

    pump.setFault();
    QCOMPARE(pump.state(), Pump::State::Fault);
    pump.start();
    QCOMPARE(pump.state(), Pump::State::Fault);
    QVERIFY(!pump.isRunning());
}

void TestPump::resetFault()
{
    Pump pump(1);

    pump.setFault();

    QCOMPARE(pump.state(), Pump::State::Fault);

    pump.resetFault();

    QCOMPARE(pump.state(), Pump::State::Stopped);
    QVERIFY(!pump.isRunning());

}

void TestPump::stopDoesNotClearFault()
{
    Pump pump(1);

    pump.setFault();
    pump.stop();

    QCOMPARE(pump.state(), Pump::State::Fault);
}

void TestPump::rpmValues()
{
    Pump pump(1);

    QCOMPARE(pump.targetRpm(), 1500.0);
    QCOMPARE(pump.actualRpm(), 0.0);

    pump.setTargetRpm(900.0);
    QCOMPARE(pump.targetRpm(), 900.0);

    pump.setActualRpm(450.0);
    QCOMPARE(pump.actualRpm(), 450.0);



}
QTEST_APPLESS_MAIN(TestPump)

#include "tst_pump.moc"