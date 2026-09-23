#include <QtTest>

#include "../src/valve.h"

class TestValve : public QObject
{
    Q_OBJECT

private slots:
    void initialState();
    void openValve();
    void closeValve();
    void faultValve();
    void resetFault();
};

void TestValve::initialState()
{
    Valve valve(1);

    QCOMPARE(valve.state(), Valve::State::Closed);
    QVERIFY(!valve.isOpen());
}

void TestValve::openValve()
{
    Valve valve(1);

    valve.open();

    QCOMPARE(valve.state(), Valve::State::Open);
    QVERIFY(valve.isOpen());
}

void TestValve::closeValve()
{
    Valve valve(1);

    valve.open();
    valve.close();

    QCOMPARE(valve.state(), Valve::State::Closed);
    QVERIFY(!valve.isOpen());
}

void TestValve::faultValve()
{
    Valve valve(1);

    valve.open();
    valve.setFault();

    QCOMPARE(valve.state(), Valve::State::Fault);

    valve.open();
    QCOMPARE(valve.state(), Valve::State::Fault);

    valve.close();
    QCOMPARE(valve.state(), Valve::State::Fault);
}

void TestValve::resetFault()
{
    Valve valve(1);

    valve.setFault();
    valve.resetFault();

    QCOMPARE(valve.state(), Valve::State::Closed);
    QVERIFY(!valve.isOpen());
}

QTEST_APPLESS_MAIN(TestValve)

#include "tst_valve.moc"
