#include <QtTest>

#include "../src/pump.h"
#include "../src/valve.h"
#include "../src/processcontroller.h"
#include "../src/eventmanager.h"

class TestProcessController : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void pumpDoesNotStartWhenValveClosed();
    void pumpStartsWhenValveOpen();
    void stopPump();
    void closingValveStopsPump();
    void pumpCanRestartAfterValveReopens();
    void highTemperatureCausesPumpFault();
    void normalTemperatureDoesNotCauseFault();
    void cannotRestartWithHighTemperature();
    void pumpStartGeneratesEvent();
    void highTemperatureGeneratesAlarm();
private:
    Pump m_pump{1};
    Valve m_valve{1};
    EventManager m_eventManager;
    ProcessController m_controller{&m_pump, &m_valve, &m_eventManager};
};

void TestProcessController::init()
{
    m_pump.resetFault();
    m_pump.stop();

    m_valve.resetFault();
    m_valve.close();
}

void TestProcessController::pumpDoesNotStartWhenValveClosed()
{
    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Stopped);
}

void TestProcessController::pumpStartsWhenValveOpen()
{
    m_valve.open();

    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);

    QCOMPARE(m_eventManager.currentSource(), QString("Pump 1"));
    QCOMPARE(m_eventManager.currentMessage(), QString("Pump started"));
    QCOMPARE(m_eventManager.currentLevel(), EventManager::Level::Info);
}

void TestProcessController::stopPump()
{
    m_valve.open();
    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);

    m_controller.stopPump();

    QCOMPARE(m_pump.state(), Pump::State::Stopped);
}

void TestProcessController::closingValveStopsPump()
{
    m_valve.open();
    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);

    m_valve.close();

    QCOMPARE(m_pump.state(), Pump::State::Stopped);
}

void TestProcessController::pumpCanRestartAfterValveReopens()
{
    m_valve.open();
    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);

    m_valve.close();

    QCOMPARE(m_pump.state(), Pump::State::Stopped);

    m_valve.open();
    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);
}

void TestProcessController::highTemperatureCausesPumpFault()
{
    m_valve.open();
    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);

    m_pump.setTemperatureFromSensor(91.0);

    QCOMPARE(m_pump.state(), Pump::State::Fault);

}
void TestProcessController::normalTemperatureDoesNotCauseFault()
{
    m_valve.open();
    m_pump.setTemperatureFromSensor(60.0);
    m_controller.startPump();
    QCOMPARE(m_pump.state(), Pump::State::Running);

    m_pump.setTemperatureFromSensor(90.0);

    QCOMPARE(m_pump.state(), Pump::State::Running);
}

void TestProcessController::cannotRestartWithHighTemperature()
{
    m_valve.open();
    m_pump.setTemperatureFromSensor(60.0);
    m_controller.startPump();
    QCOMPARE(m_pump.state(), Pump::State::Running);

    m_pump.setTemperatureFromSensor(91.0);

    QCOMPARE(m_pump.state(), Pump::State::Fault);

    m_pump.resetFault();

    QCOMPARE(m_pump.state(), Pump::State::Stopped);

    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Stopped);

    m_pump.setTemperatureFromSensor(60.0);

    m_controller.startPump();

    QCOMPARE(m_pump.state(), Pump::State::Running);
}

void TestProcessController::pumpStartGeneratesEvent()
{
    m_valve.open();

    m_controller.startPump();

    QCOMPARE(m_eventManager.currentSource(), QString("Pump 1"));
    QCOMPARE(m_eventManager.currentMessage(), QString("Pump started"));
    QCOMPARE(m_eventManager.currentLevel(), EventManager::Level::Info);
}

void TestProcessController::highTemperatureGeneratesAlarm()
{
    m_pump.setTemperatureFromSensor(60.0);
    m_valve.open();
    m_controller.startPump();

    m_pump.setTemperatureFromSensor(91.0);

    QCOMPARE(m_eventManager.currentSource(), QString("Pump 1"));
    QCOMPARE(m_eventManager.currentMessage(), QString("High temperature"));
    QCOMPARE(m_eventManager.currentLevel(), EventManager::Level::Alarm);
}

QTEST_MAIN(TestProcessController)

#include "tst_processcontroller.moc"
