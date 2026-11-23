#include <QtTest>
#include <QModbusDataUnit>
#include <QModbusTcpServer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>

#include "../src/database/databaseschema.h"
#include "../src/database/eventrepository.h"
#include "../src/database/persistenceretryqueue.h"
#include "../src/database/runrepository.h"
#include "../src/database/trendsamplerepository.h"

#include "../src/eventmanager.h"
#include "../src/operatorsession.h"
#include "../src/communication/modbuscontrollerclient.h"
#include "../src/communication/remoteprocesscontroller.h"
#include "../src/database/runpersistencecoordinator.h"
#include "../src/processeventcoordinator.h"

#include "../src/mixer.h"
#include "../src/mixingtank.h"
#include "../src/pump.h"
#include "../src/valve.h"
#include "../src/trendrecorder.h"
#include "../src/remoteprocesssynchronizer.h"

#include "controllerprotocol.h"

class TestRemoteHistoryIntegration : public QObject
{
    Q_OBJECT

private slots:
    void recordsObservedRunAcrossHmiRestart();
};

void TestRemoteHistoryIntegration::recordsObservedRunAcrossHmiRestart()
{
    QModbusTcpServer server;
    QModbusDataUnitMap registers;
    registers.insert(QModbusDataUnit::InputRegisters, QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, ControllerProtocol::RegisterCount));
    registers.insert(QModbusDataUnit::HoldingRegisters, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, ControllerProtocol::RegisterCount));

    QVERIFY(server.setMap(registers));
    server.setServerAddress(ControllerProtocol::ServerAddress);
    server.setConnectionParameter(QModbusDevice::NetworkAddressParameter, QStringLiteral("127.0.0.1"));
    server.setConnectionParameter(QModbusDevice::NetworkPortParameter, ControllerProtocol::TcpPort);

    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::ProtocolVersion), ControllerProtocol::ProtocolVersion));
    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::ControllerSessionId), 77));
    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::RunSequence), 1));
    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::RunKind), static_cast<quint16>(ControllerProtocol::RunKind::Batch)));
    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::ProcessState), static_cast<quint16>(ControllerProtocol::ProcessState::FillingWater)));
    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::WaterVolumeDecilitres), 25));
    QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::TemperatureTenthsCelsius), 310));

    QVERIFY2(server.connectDevice(), qPrintable(server.errorString()));

    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", "remote-history-integration");
    database.setDatabaseName(directory.filePath("history.sqlite"));
    QVERIFY(database.open());

    QSqlQuery pragma(database);
    QVERIFY(pragma.exec("PRAGMA foreign_keys = ON"));

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));
    QVERIFY(pragma.exec("PRAGMA query_only = ON"));

    {

        RunRepository runs(database);
        EventRepository events(database);
        TrendSampleRepository samples(database);
        PersistenceRetryQueue queue(runs, events, samples);
        EventManager eventManager;
        OperatorSession operatorSession(database);
        ModbusControllerClient client;
        RemoteProcessController remote(&client);
        Pump pump1(1);
        Pump pump2(2);
        Pump pump3(3);
        Valve valve1(1);
        Valve valve2(2);
        Valve valve3(3);
        Valve valve4(4);
        Mixer mixer;
        MixingTank tank;
        RemoteProcessSynchronizer synchronizer(&client, &pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &valve4, &mixer, &tank);
        TrendRecorder recorder(&tank);
        RunPersistenceCoordinator coordinator(eventManager, queue);
        QObject::connect(&eventManager, &EventManager::eventRecorded, &queue, [&queue](const QString &runId, qint64 timestampMs, const QString &level, const QString &source, const QString &message) { queue.submitEvent(runId, timestampMs, level, source, message); });

        coordinator.watchBatchStart(remote, operatorSession);

        QObject::connect(&recorder, &TrendRecorder::sampleRecorded, &queue, [&queue](const QString &runId, qint64 timestampMs, double volumeL, double temperatureC) { queue.submitSample(runId, timestampMs, volumeL, temperatureC); });
        coordinator.watchTrendRecording(remote, recorder);

        ProcessEventCoordinator eventCoordinator(eventManager);
        eventCoordinator.watchBatchId(remote);
        eventCoordinator.watchBatchState(remote, operatorSession);
        QSignalSpy stateSpy(&remote, &RemoteProcessController::stateChanged);

        QVERIFY(client.connectToController(QStringLiteral("127.0.0.1")));
        QTRY_COMPARE_WITH_TIMEOUT(remote.batchId(), QStringLiteral("esp-77-1"), 5000);
        QTRY_VERIFY_WITH_TIMEOUT(stateSpy.count() > 0, 5000);
        QCOMPARE(remote.state(), static_cast<int>(ControllerProtocol::ProcessState::FillingWater));
        QCOMPARE(eventManager.eventCount(), 0);
        QVERIFY(server.setData(QModbusDataUnit::InputRegisters, ControllerProtocol::address(ControllerProtocol::InputRegister::ProcessState), static_cast<quint16>(ControllerProtocol::ProcessState::Paused)));
        QTRY_COMPARE_WITH_TIMEOUT(eventManager.eventCount(), 1, 5000);
        QCOMPARE(eventManager.eventMessage(0), QStringLiteral("Hardware batch paused"));
        QVERIFY(queue.hasPendingWrites());
        QVERIFY(pragma.exec("PRAGMA query_only = OFF"));
        QTRY_VERIFY_WITH_TIMEOUT(!queue.hasPendingWrites(), 5000);
        const auto expectedSampleSaved = [&database]() { QSqlQuery query(database); return query.exec("SELECT 1 FROM trend_samples WHERE run_id = 'esp-77-1' AND volume_l = 2.5 AND temperature_c = 31.0 LIMIT 1") && query.next(); };
        QTRY_VERIFY_WITH_TIMEOUT(expectedSampleSaved(), 5000);
    }
    QSqlQuery savedEvent(database);
    QVERIFY(savedEvent.exec("SELECT run_id, source, message FROM events WHERE run_id = 'esp-77-1'"));
    QVERIFY(savedEvent.next());
    QCOMPARE(savedEvent.value(0).toString(), QStringLiteral("esp-77-1"));
    QCOMPARE(savedEvent.value(1).toString(), QStringLiteral("BATCH"));
    QCOMPARE(savedEvent.value(2).toString(), QStringLiteral("Hardware batch paused"));
    QVERIFY(!savedEvent.next());
    savedEvent.finish();

    QSqlQuery savedRun(database);
    QVERIFY(savedRun.exec("SELECT kind, batch_operator_user_id FROM runs WHERE run_id = 'esp-77-1'"));
    QVERIFY(savedRun.next());
    QCOMPARE(savedRun.value(0).toString(), QStringLiteral("BATCH"));
    QVERIFY(savedRun.value(1).isNull());
    QVERIFY(!savedRun.next());
    savedRun.finish();
    database.close();
    QVERIFY(database.open());
    QVERIFY(pragma.exec("PRAGMA foreign_keys = ON"));

    {
        RunRepository runs(database);
        EventRepository events(database);
        TrendSampleRepository samples(database);
        PersistenceRetryQueue queue(runs, events, samples);
        EventManager eventManager;
        OperatorSession operatorSession(database);
        ModbusControllerClient client;
        RemoteProcessController remote(&client);
        RunPersistenceCoordinator coordinator(eventManager, queue);

        coordinator.watchBatchStart(remote, operatorSession);

        ProcessEventCoordinator eventCoordinator(eventManager);
        eventCoordinator.watchBatchId(remote);
        eventCoordinator.watchBatchState(remote, operatorSession);
        QSignalSpy stateSpy(&remote, &RemoteProcessController::stateChanged);

        QVERIFY(client.connectToController(QStringLiteral("127.0.0.1")));
        QTRY_COMPARE_WITH_TIMEOUT(remote.batchId(), QStringLiteral("esp-77-1"), 5000);
        QTRY_VERIFY_WITH_TIMEOUT(stateSpy.count() > 0, 5000);
        QCOMPARE(remote.state(), static_cast<int>(ControllerProtocol::ProcessState::Paused));
        QCOMPARE(eventManager.eventCount(), 0);
        QVERIFY(!queue.hasPendingWrites());
    }

    QSqlQuery count(database);
    QVERIFY(count.exec("SELECT COUNT(*) FROM runs WHERE run_id = 'esp-77-1'"));
    QVERIFY(count.next());
    QCOMPARE(count.value(0).toInt(), 1);
    QSqlQuery eventCount(database);
    QVERIFY(eventCount.exec("SELECT COUNT(*) FROM events WHERE run_id = 'esp-77-1'"));
    QVERIFY(eventCount.next());
    QCOMPARE(eventCount.value(0).toInt(), 1);
}

QTEST_GUILESS_MAIN(TestRemoteHistoryIntegration)
#include "tst_remotehistoryintegration.moc"
