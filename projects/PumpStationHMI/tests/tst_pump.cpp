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
};

void TestPump::initialState()
{
    Pump pump;

    QCOMPARE(pump.state(), Pump::State::Stopped);

    QVERIFY(!pump.isRunning());
}

void TestPump::startPump()
{
    Pump pump;

    pump.start();

    QVERIFY(pump.isRunning());
}
void TestPump::stopPump()
{
    Pump pump;

    pump.start();
    pump.stop();

    QVERIFY(!pump.isRunning());
}

void TestPump::faultState()
{
    Pump pump;

    pump.setFault();
    QCOMPARE(pump.state(), Pump::State::Fault);
    pump.start();
    QCOMPARE(pump.state(), Pump::State::Fault);
    QVERIFY(!pump.isRunning());
}

void TestPump::resetFault()
{
    Pump pump;

    pump.setFault();

    QCOMPARE(pump.state(), Pump::State::Fault);

    pump.resetFault();

    QCOMPARE(pump.state(), Pump::State::Stopped);
    QVERIFY(!pump.isRunning());

}

void TestPump::stopDoesNotClearFault()
{
    Pump pump;

    pump.setFault();
    pump.stop();

    QCOMPARE(pump.state(), Pump::State::Fault);
}

QTEST_APPLESS_MAIN(TestPump)

#include "tst_pump.moc"