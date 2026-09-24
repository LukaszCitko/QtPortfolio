
#include <QtTest>

#include "../src/batchcontroller.h"
#include "../src/mixingtank.h"
#include "../src/pump.h"
#include "../src/valve.h"
#include "../src/processsimulator.h"
#include "../src/mixer.h"

class TestBatchController : public QObject
{
    Q_OBJECT

private slots:
    //batch process
    void startBatchStartsWaterFilling();
    void waterFillingAutomaticallyChangesToConcentrateDosing();
    void concentrateDosingAutomaticallyChangesToTemperatureCheck();
    void correctTemperatureStartsMixing();
    void incorrectTemperatureKeepsTemperatureCheck();
    void mixingFinishesAfterSixtySeconds();
    void readyForTransferStartsTransferring();
    void transferringAutomaticallyFinishesBatch();

    void emergencyDrainStopsAllProcessDevices();
    void mixerFaultBlocksConcentrateDosing();
    void mixerStartsBeforeConcentrateDosing();
    void emergencyDrainStopsMixer();
    void emergencyDrainClosesAllProcessValves();
    void emergencyDrainOpensDrainValve();

    //pause & resume
    void pauseDuringWaterFilling();
    void resumeAfterWaterFillingPause();
    void pauseWhenIdleDoesNothing();
    void pauseDuringConcentrateDosing();
    void resumeAfterConcentrateDosingPause();
    void pauseDuringMixing();
    void resumeAfterMixingPause();
    void pauseDuringTransferring();
    void resumeAfterTransferringPause();
    void pauseDuringTemperatureCheck();
    void resumeAfterTemperatureCheckPause();
    void pauseWhenReadyForTransferDoesNothing();
    void pauseWhenCompleteDoesNothing();

    //alarms & faults
    void mixerFaultDuringConcentrateDosing();
};

void TestBatchController::startBatchStartsWaterFilling()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    QCOMPARE(controller.stateText(), QString("IDLE"));

    controller.startBatch();

    QCOMPARE(
        controller.stateText(),
        QString("FILLING WATER"));

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
}

void TestBatchController::waterFillingAutomaticallyChangesToConcentrateDosing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    Valve valve4(4);
    Mixer mixer;

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();

    QCOMPARE(
        controller.stateText(),
        QString("FILLING WATER"));

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    simulator.simulateStep(70.0);

    QCOMPARE(
        controller.stateText(),
        QString("DOSING CONCENTRATE"));

    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());

    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());

    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
}
void TestBatchController::concentrateDosingAutomaticallyChangesToTemperatureCheck() {

    Pump pump1(1);
    Valve valve1(1);
    Pump pump2(2);
    Valve valve2(2);
    Pump pump3(3);
    Valve valve3(3);
    Valve valve4(4);
    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();
    simulator.simulateStep(70.0);
    QCOMPARE( controller.stateText(), QString("DOSING CONCENTRATE"));
    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());
    simulator.simulateStep(30.0);
    QCOMPARE( controller.stateText(), QString("TEMPERATURE CHECK"));
    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);
    QCOMPARE(tank.volume(), 100.0);
}

void TestBatchController::correctTemperatureStartsMixing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3)
        ;
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    QCOMPARE(tank.volume(), 100.0);

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));
}

void TestBatchController::incorrectTemperatureKeepsTemperatureCheck()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(55.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(65.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));
}


void TestBatchController::mixingFinishesAfterSixtySeconds()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);

    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(29.9);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(0.1);

    QCOMPARE(
        controller.stateText(),
        QString("READY FOR TRANSFER"));
}

void TestBatchController::readyForTransferStartsTransferring()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);
    Valve valve4(4);
    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    controller.simulateStep(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("READY FOR TRANSFER"));

    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());

    controller.startTransfer();

    QCOMPARE(
        controller.stateText(),
        QString("TRANSFERRING"));

    QVERIFY(pump3.isRunning());
    QVERIFY(valve3.isOpen());
}

void TestBatchController::transferringAutomaticallyFinishesBatch()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);
    Valve valve4(4);
    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    controller.startBatch();

    // Fill 70 L of water.
    simulator.simulateStep(70.0);

    // Dose 30 L of concentrate.
    simulator.simulateStep(30.0);

    QCOMPARE(
        controller.stateText(),
        QString("TEMPERATURE CHECK"));

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    // Finish 60-second mixing.
    controller.simulateStep(60.0);

    QCOMPARE(
        controller.stateText(),
        QString("READY FOR TRANSFER"));

    controller.startTransfer();

    QCOMPARE(
        controller.stateText(),
        QString("TRANSFERRING"));

    QVERIFY(pump3.isRunning());
    QVERIFY(valve3.isOpen());

    // Transfer 100 L at 2 L/s.
    simulator.simulateStep(50.0);

    QCOMPARE(tank.volume(), 0.0);

    QCOMPARE(
        controller.stateText(),
        QString("COMPLETE"));

    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}


void TestBatchController::emergencyDrainStopsAllProcessDevices()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Pump pump3(3);
    Valve valve3(3);
    Valve valve4(4);
    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    controller.startBatch();

    simulator.simulateStep(20.0);

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    controller.emergencyDrain();

    QVERIFY(!pump1.isRunning());
    QVERIFY(!pump2.isRunning());
    QVERIFY(!pump3.isRunning());

    QVERIFY(!valve1.isOpen());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!valve3.isOpen());

    QCOMPARE(
        controller.stateText(),
        QString("IDLE"));
}

void TestBatchController::mixerStartsBeforeConcentrateDosing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Mixer mixer;

    Pump pump3(3);
    Valve valve3(3);
    Valve valve4(4);
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);

    QCOMPARE(controller.state(),
             BatchController::State::DosingConcentrate);

    QVERIFY(mixer.isRunning());
    QVERIFY(valve2.isOpen());
    QVERIFY(pump2.isRunning());
}
void TestBatchController::mixerFaultBlocksConcentrateDosing()
{
    Pump pump1(1);
    Valve valve1(1);

    Pump pump2(2);
    Valve valve2(2);

    Mixer mixer;

    Pump pump3(3);
    Valve valve3(3);
    Valve valve4(4);

    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    mixer.setFault();

    controller.startBatch();

    simulator.simulateStep(70.0);

    QVERIFY(!mixer.isRunning());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!pump2.isRunning());
}

void TestBatchController::emergencyDrainStopsMixer()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();
    mixer.start();

    QVERIFY(mixer.isRunning());

    controller.emergencyDrain();

    QVERIFY(!mixer.isRunning());
}

void TestBatchController::emergencyDrainClosesAllProcessValves()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    valve1.open();
    valve2.open();
    valve3.open();

    QVERIFY(valve1.isOpen());
    QVERIFY(valve2.isOpen());
    QVERIFY(valve3.isOpen());

    controller.emergencyDrain();

    QVERIFY(!valve1.isOpen());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::emergencyDrainOpensDrainValve()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    QVERIFY(!valve4.isOpen());

    controller.emergencyDrain();

    QVERIFY(valve4.isOpen());
}

void TestBatchController::pauseDuringWaterFilling()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    controller.startBatch();

    simulator.simulateStep(10.0);

    QCOMPARE(controller.state(), BatchController::State::FillingWater);
    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());
    QCOMPARE(tank.waterVolume(), 10.0);

    controller.pause();

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());

    simulator.simulateStep(10.0);

    QCOMPARE(tank.waterVolume(), 10.0);
}
void TestBatchController::resumeAfterWaterFillingPause()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    controller.startBatch();

    simulator.simulateStep(10.0);

    controller.pause();

    QCOMPARE(tank.waterVolume(), 10.0);

    controller.resume();

    QCOMPARE(controller.state(), BatchController::State::FillingWater);
    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    simulator.simulateStep(10.0);

    QCOMPARE(tank.waterVolume(), 20.0);
}

void TestBatchController::pauseWhenIdleDoesNothing()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Idle);

    QVERIFY(!pump1.isRunning());
    QVERIFY(!pump2.isRunning());
    QVERIFY(!pump3.isRunning());
}

void TestBatchController::pauseDuringConcentrateDosing()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::DosingConcentrate);

    QVERIFY(mixer.isRunning());
    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());

    simulator.simulateStep(10.0);

    QCOMPARE(tank.concentrateVolume(), 10.0);

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());

    // Mixer must remain running during pause.
    QVERIFY(mixer.isRunning());

    simulator.simulateStep(10.0);

    // No additional concentrate should be added while paused.
    QCOMPARE(tank.concentrateVolume(), 10.0);
}
void TestBatchController::resumeAfterConcentrateDosingPause()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(10.0);

    controller.pause();

    QCOMPARE(tank.concentrateVolume(), 10.0);
    QVERIFY(mixer.isRunning());

    controller.resume();

    QCOMPARE(
        controller.state(),
        BatchController::State::DosingConcentrate);

    QVERIFY(mixer.isRunning());
    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());

    simulator.simulateStep(10.0);

    QCOMPARE(tank.concentrateVolume(), 20.0);
}

void TestBatchController::pauseDuringMixing()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    QVERIFY(mixer.isRunning());

    controller.simulateStep(20.0);

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QVERIFY(!mixer.isRunning());

    // The mixing timer must be stopped while paused.
    controller.simulateStep(20.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);
}

void TestBatchController::resumeAfterMixingPause()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    controller.simulateStep(20.0);

    controller.pause();

    QVERIFY(!mixer.isRunning());

    controller.simulateStep(20.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    controller.resume();

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    QVERIFY(mixer.isRunning());

    controller.simulateStep(39.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    controller.simulateStep(1.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::ReadyForTransfer);

    QVERIFY(!mixer.isRunning());
}

void TestBatchController::pauseDuringTransferring()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    // Prepare a ready product.
    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    controller.simulateStep(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::ReadyForTransfer);

    controller.startTransfer();

    QVERIFY(pump3.isRunning());
    QVERIFY(valve3.isOpen());

    simulator.simulateStep(5.0);

    const double volumeBeforePause = tank.volume();

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());

    simulator.simulateStep(10.0);

    QCOMPARE(tank.volume(), volumeBeforePause);
}

void TestBatchController::resumeAfterTransferringPause()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    controller.simulateStep(60.0);

    controller.startTransfer();

    simulator.simulateStep(5.0);

    const double volumeBeforePause = tank.volume();

    controller.pause();

    controller.resume();

    QCOMPARE(
        controller.state(),
        BatchController::State::Transferring);

    QVERIFY(pump3.isRunning());
    QVERIFY(valve3.isOpen());

    simulator.simulateStep(5.0);

    QCOMPARE(tank.volume(), volumeBeforePause - 10.0);
}

void TestBatchController::pauseDuringTemperatureCheck()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    tank.setTemperature(60.0);

    // 70 L water.
    tank.addWater(70.0);

    // 30 L concentrate.
    tank.addConcentrate(30.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::TemperatureCheck);

    QVERIFY(mixer.isRunning());

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QVERIFY(mixer.isRunning());
    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
}

void TestBatchController::resumeAfterTemperatureCheckPause()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    tank.addWater(70.0);
    tank.addConcentrate(30.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::TemperatureCheck);

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QVERIFY(mixer.isRunning());

    controller.resume();

    QCOMPARE(
        controller.state(),
        BatchController::State::TemperatureCheck);

    QVERIFY(mixer.isRunning());

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    QVERIFY(mixer.isRunning());
}
void TestBatchController::pauseWhenReadyForTransferDoesNothing()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    tank.addWater(70.0);
    tank.addConcentrate(30.0);
    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    controller.simulateStep(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::ReadyForTransfer);

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::ReadyForTransfer);

    QVERIFY(!mixer.isRunning());
    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::pauseWhenCompleteDoesNothing()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    tank.addWater(70.0);
    tank.addConcentrate(30.0);
    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    controller.simulateStep(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::ReadyForTransfer);

    controller.startTransfer();

    QCOMPARE(
        controller.state(),
        BatchController::State::Transferring);

    tank.removeProduct(100.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Complete);

    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Complete);

    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::mixerFaultDuringConcentrateDosing()
{
    Pump pump1(1);
    Pump pump2(2);
    Pump pump3(3);

    Valve valve1(1);
    Valve valve2(2);
    Valve valve3(3);
    Valve valve4(4);

    Mixer mixer;
    MixingTank tank;

    ProcessSimulator simulator(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &tank);

    BatchController controller(
        &pump1,
        &valve1,
        &pump2,
        &valve2,
        &pump3,
        &valve3,
        &valve4,
        &mixer,
        &tank);

    mixer.connect();

    controller.startBatch();

    tank.addWater(70.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::DosingConcentrate);

    QVERIFY(mixer.isRunning());
    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());

    mixer.setFault();

    QCOMPARE(
        mixer.state(),
        Mixer::State::Fault);

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QVERIFY(!pump1.isRunning());
    QVERIFY(!pump2.isRunning());
    QVERIFY(!pump3.isRunning());

    QVERIFY(!valve1.isOpen());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!valve3.isOpen());

    QVERIFY(valve4.isOpen() == false);
}

QTEST_MAIN(TestBatchController)

#include "tst_batchcontroller.moc"
