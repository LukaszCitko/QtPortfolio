
#include <QtTest>
#include <QObject>
#include <QPointer>

#include "../src/batchcontroller.h"
#include "../src/mixingtank.h"
#include "../src/pump.h"
#include "../src/valve.h"
#include "../src/processsimulator.h"
#include "../src/mixer.h"

//MACRO to add devices to tests...
#define TEST_DEVICES \
\
    Pump pump1(1); Valve valve1(1);\
    Pump pump2(2); Valve valve2(2);\
    Pump pump3(3); Valve valve3(3);\
    Valve valve4(4);\
    Mixer mixer; MixingTank tank;\
    ProcessSimulator simulator(\
        &pump1,&valve1,&pump2,&valve2,\
        &pump3,&valve3,&tank);\
    BatchController controller(\
        &pump1, &valve1, &pump2, &valve2,\
        &pump3, &valve3, &valve4,\
        &mixer, &tank);


class TestBatchController : public QObject
{
    Q_OBJECT

private slots:
    //batch process
    void tryStartBatchChecksConditions();
    void startBatchStartsWaterFilling();
    void waterFillingAutomaticallyChangesToConcentrateDosing();
    void concentrateDosingAutomaticallyChangesToTemperatureCheck();
    void correctTemperatureStartsMixing();
    void incorrectTemperatureKeepsTemperatureCheck();
    void mixingFinishesAfterFifteenSeconds();
    void readyForTransferStartsTransferring();
    void transferringAutomaticallyFinishesBatch();
    void temperatureSetBeforeCheckStartsMixing();
    void completedBatchCanReturnToPrecheck();

    void emergencyDrainStopsAllProcessDevices();
    void mixerFaultBlocksConcentrateDosing();
    void mixerStartsBeforeConcentrateDosing();
    void emergencyDrainStopsMixer();
    void emergencyDrainClosesAllProcessValves();
    void emergencyDrainOpensDrainValve();
    //pause & resume
    void stageIndexRemainsOnPausedStep();
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
    void resumeAfterPump1Fault();
    void resumeAfterPump2Fault();
    void resumeWhilePump2FaultActive();
    void resumeWhilePump1FaultActive();
    void resumeWhilePump3FaultActive();
    void resumeAfterPump3Fault();
    void startTransferAfterPump3FaultReset();

    //alarms & faults
    void mixerFaultDuringConcentrateDosing();
    void resumeAfterMixerFault();
    void pump1FaultDuringWaterFilling();
    void pump2FaultDuringConcentrateDosing();
    void pump3FaultDuringTransfer();
    void startTransferWhilePump3FaultActive();
    void startBatchWhilePump1FaultActive();
    void pump2FaultBeforeConcentrateDosing();
    void resumeDosingAfterPreexistingPump2Fault();

    //stop reason
    void operatorPauseSetsStopReason();
};

void TestBatchController::tryStartBatchChecksConditions()
{
    TEST_DEVICES

    QVERIFY(!controller.tryStartBatch("Operator A"));
    QCOMPARE(controller.state(), BatchController::State::Idle);

    mixer.connect();

    QVERIFY(!controller.tryStartBatch("  "));

    valve4.open();
    QVERIFY(!controller.tryStartBatch("Operator A"));
    valve4.close();

    pump2.setFault();
    QVERIFY(!controller.tryStartBatch("Operator A"));
    pump2.resetFault();

    QVERIFY(controller.tryStartBatch("Operator A"));
    QCOMPARE(controller.state(), BatchController::State::FillingWater);
    QVERIFY(pump1.isRunning());
}

void TestBatchController::startBatchStartsWaterFilling()
{
    TEST_DEVICES

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
    TEST_DEVICES

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

    TEST_DEVICES
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
    TEST_DEVICES
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
    TEST_DEVICES

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


void TestBatchController::mixingFinishesAfterFifteenSeconds()
{
    TEST_DEVICES

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

    controller.simulateStep(7.0);

    QCOMPARE(
        controller.stateText(),
        QString("MIXING"));

    controller.simulateStep(7.9);

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
    TEST_DEVICES

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
    TEST_DEVICES
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
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

    mixer.connect();
    mixer.start();

    QVERIFY(mixer.isRunning());

    controller.emergencyDrain();

    QVERIFY(!mixer.isRunning());
}

void TestBatchController::emergencyDrainClosesAllProcessValves()
{
    TEST_DEVICES


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
    TEST_DEVICES

    QVERIFY(!valve4.isOpen());

    controller.emergencyDrain();

    QVERIFY(valve4.isOpen());
}

void TestBatchController::pauseDuringWaterFilling()
{
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    QVERIFY(mixer.isRunning());

    controller.simulateStep(5.0);

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
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);

    tank.setTemperature(60.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    controller.simulateStep(5.0);

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

    controller.simulateStep(9.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::Mixing);

    controller.simulateStep(1.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::ReadyForTransfer);

    QVERIFY(mixer.isRunning());
}

void TestBatchController::pauseDuringTransferring()
{
    TEST_DEVICES

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
    TEST_DEVICES

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
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    tank.setTemperature(55.0);

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
    TEST_DEVICES

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
    TEST_DEVICES

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

    QVERIFY(mixer.isRunning());
    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::pauseWhenCompleteDoesNothing()
{
    TEST_DEVICES

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
    TEST_DEVICES

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

    QCOMPARE(
        controller.stopReason(),
        BatchController::StopReason::MixerFault);

    QCOMPARE(
        controller.stopReasonText(),
        QString("MIXER FAULT"));

    QVERIFY(!pump1.isRunning());
    QVERIFY(!pump2.isRunning());
    QVERIFY(!pump3.isRunning());

    QVERIFY(!valve1.isOpen());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!valve3.isOpen());

    QVERIFY(valve4.isOpen() == false);
}

void TestBatchController::resumeAfterMixerFault()
{
    TEST_DEVICES

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

    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());

    // Operator fixes the mixer fault.
    mixer.resetFault();

    QCOMPARE(
        mixer.state(),
        Mixer::State::Stopped);

    controller.resume();

    QCOMPARE(
        controller.state(),
        BatchController::State::DosingConcentrate);

    QVERIFY(mixer.isRunning());
    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());
}
void TestBatchController::operatorPauseSetsStopReason()
{
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    QCOMPARE(
        controller.state(),
        BatchController::State::FillingWater);

    QCOMPARE(
        controller.stopReason(),
        BatchController::StopReason::None);

    controller.pause();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QCOMPARE(
        controller.stopReason(),
        BatchController::StopReason::OperatorPause);

    QCOMPARE(
        controller.stopReasonText(),
        QString("OPERATOR PAUSE"));
}

void TestBatchController::pump1FaultDuringWaterFilling()
{
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    QCOMPARE(
        controller.state(),
        BatchController::State::FillingWater);

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());

    pump1.setFault();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QCOMPARE(
        controller.stopReason(),
        BatchController::StopReason::Pump1Fault);

    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());
}
void TestBatchController::resumeAfterPump1Fault()
{
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    QCOMPARE(
        controller.state(),
        BatchController::State::FillingWater);

    pump1.setFault();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QCOMPARE(
        controller.stopReason(),
        BatchController::StopReason::Pump1Fault);

    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());

    pump1.resetFault();

    controller.resume();

    QCOMPARE(
        controller.state(),
        BatchController::State::FillingWater);

    QVERIFY(pump1.isRunning());
    QVERIFY(valve1.isOpen());
}
void TestBatchController::pump2FaultDuringConcentrateDosing()
{
    TEST_DEVICES

    mixer.connect();

    controller.startBatch();

    // 70 L water -> start concentrate dosing.
    tank.addWater(70.0);

    QCOMPARE(
        controller.state(),
        BatchController::State::DosingConcentrate);

    QVERIFY(pump2.isRunning());
    QVERIFY(valve2.isOpen());
    QVERIFY(mixer.isRunning());

    // Simulate Pump 2 fault.
    pump2.setFault();

    QCOMPARE(
        controller.state(),
        BatchController::State::Paused);

    QCOMPARE(
        controller.stopReason(),
        BatchController::StopReason::Pump2Fault);

    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!mixer.isRunning());
}

void TestBatchController::resumeAfterPump2Fault()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    tank.addWater(70.0);

    QCOMPARE(controller.state(),
             BatchController::State::DosingConcentrate);

    pump2.setFault();

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stopReason(),
             BatchController::StopReason::Pump2Fault);
    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!mixer.isRunning());

    pump2.resetFault();
    controller.resume();

    QCOMPARE(controller.state(),
             BatchController::State::DosingConcentrate);
    QVERIFY(mixer.isRunning());
    QVERIFY(valve2.isOpen());
    QVERIFY(pump2.isRunning());
}

void TestBatchController::resumeWhilePump2FaultActive()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    tank.addWater(70.0);

    pump2.setFault();

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stopReason(),
             BatchController::StopReason::Pump2Fault);

    controller.resume();  // Fault has not been reset.

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!mixer.isRunning());
}
void TestBatchController::resumeWhilePump1FaultActive()
{
    TEST_DEVICES

    controller.startBatch();
    pump1.setFault();

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stopReason(),
             BatchController::StopReason::Pump1Fault);

    controller.resume();  // Fault still continue

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());
}

void TestBatchController::pump3FaultDuringTransfer()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);
    tank.setTemperature(60.0);
    controller.simulateStep(60.0);
    controller.startTransfer();

    QCOMPARE(controller.state(), BatchController::State::Transferring);
    QVERIFY(pump3.isRunning());

    pump3.setFault();

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stopReason(),
             BatchController::StopReason::Pump3Fault);
    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::resumeWhilePump3FaultActive()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);
    tank.setTemperature(60.0);
    controller.simulateStep(60.0);
    controller.startTransfer();
    pump3.setFault();

    controller.resume();  // The fault is still active.

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::resumeAfterPump3Fault()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);
    tank.setTemperature(60.0);
    controller.simulateStep(60.0);
    controller.startTransfer();
    pump3.setFault();

    QCOMPARE(controller.state(), BatchController::State::Paused);

    pump3.resetFault();
    controller.resume();

    QCOMPARE(controller.state(), BatchController::State::Transferring);
    QVERIFY(pump3.isRunning());
    QVERIFY(valve3.isOpen());
}

void TestBatchController::startTransferWhilePump3FaultActive()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);
    tank.setTemperature(60.0);
    controller.simulateStep(60.0);

    QCOMPARE(controller.state(),
             BatchController::State::ReadyForTransfer);

    pump3.setFault();
    controller.startTransfer();

    QCOMPARE(controller.state(),
             BatchController::State::ReadyForTransfer);
    QVERIFY(!pump3.isRunning());
    QVERIFY(!valve3.isOpen());
}

void TestBatchController::startBatchWhilePump1FaultActive()
{
    TEST_DEVICES

    pump1.setFault();
    controller.startBatch();

    QCOMPARE(controller.state(), BatchController::State::Idle);
    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());
}

void TestBatchController::pump2FaultBeforeConcentrateDosing()
{
    TEST_DEVICES

    mixer.connect();
    pump2.setFault();

    controller.startBatch();
    QCOMPARE(controller.state(), BatchController::State::FillingWater);

    tank.addWater(70.0);

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stopReason(),
             BatchController::StopReason::Pump2Fault);

    QVERIFY(!pump1.isRunning());
    QVERIFY(!valve1.isOpen());
    QVERIFY(!pump2.isRunning());
    QVERIFY(!valve2.isOpen());
    QVERIFY(!mixer.isRunning());
}

void TestBatchController::startTransferAfterPump3FaultReset()
{
    TEST_DEVICES

    mixer.connect();
    controller.startBatch();
    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);
    tank.setTemperature(60.0);
    controller.simulateStep(60.0);

    QCOMPARE(controller.state(),
             BatchController::State::ReadyForTransfer);

    pump3.setFault();
    controller.startTransfer();

    QCOMPARE(controller.state(),
             BatchController::State::ReadyForTransfer);
    QVERIFY(!valve3.isOpen());

    pump3.resetFault();
    controller.startTransfer();

    QCOMPARE(controller.state(),
             BatchController::State::Transferring);
    QVERIFY(valve3.isOpen());
    QVERIFY(pump3.isRunning());
}

void TestBatchController::resumeDosingAfterPreexistingPump2Fault()
{
    TEST_DEVICES

    mixer.connect();
    pump2.setFault();

    controller.startBatch();
    tank.addWater(70.0);

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stopReason(),
             BatchController::StopReason::Pump2Fault);

    pump2.resetFault();
    controller.resume();

    QCOMPARE(controller.state(),
             BatchController::State::DosingConcentrate);
    QVERIFY(mixer.isRunning());
    QVERIFY(valve2.isOpen());
    QVERIFY(pump2.isRunning());
    QVERIFY(!pump1.isRunning());
}

void TestBatchController::stageIndexRemainsOnPausedStep()
{
    TEST_DEVICES

    mixer.connect();

    QCOMPARE(controller.stageIndex(), 0);

    controller.startBatch();
    QCOMPARE(controller.stageIndex(), 1);

    tank.addWater(70.0);
    QCOMPARE(controller.stageIndex(), 2);

    controller.pause();

    QCOMPARE(controller.state(), BatchController::State::Paused);
    QCOMPARE(controller.stageIndex(), 2);
}

void TestBatchController::temperatureSetBeforeCheckStartsMixing()
{
    TEST_DEVICES

    mixer.connect();
    tank.setTemperature(60.0);

    QVERIFY(controller.tryStartBatch("Operator 01"));

    simulator.simulateStep(70.0);
    QCOMPARE(controller.state(), BatchController::State::DosingConcentrate);

    simulator.simulateStep(30.0);

    QCOMPARE(controller.state(), BatchController::State::Mixing);
    QCOMPARE(tank.state(), MixingTank::State::Mixing);
    QVERIFY(mixer.isRunning());
}

void TestBatchController::completedBatchCanReturnToPrecheck()
{
    TEST_DEVICES

    mixer.connect();
    QVERIFY(controller.tryStartBatch("Operator 01"));

    simulator.simulateStep(70.0);
    simulator.simulateStep(30.0);
    tank.setTemperature(60.0);
    controller.simulateStep(15.0);

    QCOMPARE(controller.state(), BatchController::State::ReadyForTransfer);
    QVERIFY(mixer.isRunning());

    controller.startTransfer();
    QCOMPARE(controller.state(), BatchController::State::Transferring);
    QVERIFY(!mixer.isRunning());

    simulator.simulateStep(50.0);
    QCOMPARE(controller.state(), BatchController::State::Complete);
    QCOMPARE(tank.volume(), 0.0);
    QCOMPARE(tank.waterVolume(), 70.0);
    QCOMPARE(tank.concentrateVolume(), 30.0);

    QVERIFY(controller.prepareNextBatch());

    QCOMPARE(controller.state(), BatchController::State::Idle);
    QCOMPARE(controller.stageIndex(), 0);
    QCOMPARE(tank.state(), MixingTank::State::Empty);
    QCOMPARE(tank.volume(), 0.0);
    QCOMPARE(tank.waterVolume(), 0.0);
    QCOMPARE(tank.concentrateVolume(), 0.0);
    QCOMPARE(tank.temperature(), 25.0);

    QVERIFY(controller.tryStartBatch("Operator 02"));
    QCOMPARE(controller.state(), BatchController::State::FillingWater);
    QVERIFY(pump1.isRunning());
}


QTEST_MAIN(TestBatchController)

#include "tst_batchcontroller.moc"
