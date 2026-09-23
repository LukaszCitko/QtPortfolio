#include <QtTest>

#include "../src/pump.h"
#include "../src/processsimulator.h"

class TestProcessSimulator : public QObject
{
    Q_OBJECT

private slots:
    void reachesTargetRpm();
    void pumpsStop();
    void restartPump();
    void faultStopsPump();
};

void TestProcessSimulator::reachesTargetRpm()
{
    Pump pump(1);

    pump.start();
    pump.setTargetRpm(200.0);

    ProcessSimulator simulator(&pump);

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 200.0, 1100);
}


void TestProcessSimulator::pumpsStop()
{
    Pump pump(1);

    pump.start();
    pump.setTargetRpm(500.0);

    ProcessSimulator simulator(&pump);

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 500.0, 1100);

    pump.stop();

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 0.0, 1100);

    QCOMPARE(pump.targetRpm(), 500.0);
}

void TestProcessSimulator::restartPump()
{
    Pump pump(1);

    pump.start();
    pump.setTargetRpm(500.0);

    ProcessSimulator simulator(&pump);

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 500.0, 1100);

    pump.stop();

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 0.0, 1100);

    pump.start();

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 500.0, 1100);
}

void TestProcessSimulator::faultStopsPump()
{
    Pump pump(1);

    pump.start();
    pump.setTargetRpm(500.0);

    ProcessSimulator simulator(&pump);

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 500.0, 1100);

    pump.setFault();

    QTRY_COMPARE_WITH_TIMEOUT(pump.actualRpm(), 0.0, 1100);

    QCOMPARE(pump.targetRpm(), 500.0);
}


QTEST_MAIN(TestProcessSimulator)

#include "tst_processsimulator.moc"
