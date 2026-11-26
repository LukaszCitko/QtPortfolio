#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlDatabase>
#include <QTimer>

#include "batchcontroller.h"
#include "drainservice.h"
#include "eventmanager.h"
#include "faultresetservice.h"
#include "hardwaremode.h"
#include "mixer.h"
#include "mixingtank.h"
#include "operatorsession.h"
#include "processconfiguration.h"
#include "processeventcoordinator.h"
#include "processsimulator.h"
#include "pump.h"
#include "remoteprocesssynchronizer.h"
#include "trendrecorder.h"
#include "usermanagementservice.h"
#include "valve.h"

#include "database/applicationdatabase.h"
#include "database/eventhistorymodel.h"
#include "database/eventrepository.h"
#include "database/historypersistence.h"
#include "database/persistenceretryqueue.h"
#include "database/runlistmodel.h"
#include "database/runpersistencecoordinator.h"
#include "database/runrepository.h"
#include "database/trendhistorysource.h"
#include "database/trendsamplerepository.h"
#include "database/userlistmodel.h"
#include "database/userrepository.h"

#include "communication/modbuscontrollerclient.h"
#include "communication/remoteprocesscontroller.h"
#include "communication/remotecommandfeedback.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser commandLineParser;
    commandLineParser.setApplicationDescription(QStringLiteral("Mixing Station HMI"));
    commandLineParser.addHelpOption();

    QCommandLineOption controllerHostOption(QStringLiteral("controller-host"), QStringLiteral("Connect the HMI to a Modbus TCP process controller."), QStringLiteral("address"));
    commandLineParser.addOption(controllerHostOption);
    commandLineParser.process(app);

    const QString controllerHost = commandLineParser.value(controllerHostOption).trimmed();
    const bool hardwareMode = !controllerHost.isEmpty();


    QSqlDatabase database;
    QString databaseError;

    if (!prepareApplicationDatabase(database, &databaseError)) {
        qCritical() << databaseError;
        return 1;
    }

    qInfo() << "SQLite database opened at" << database.databaseName();

    UserListModel userListModel(database);
    UserRepository userRepository(database);

    QString usersError;
    if (!userListModel.reload(&usersError))
    {
        qCritical() << "Cannot load users:" << usersError;
        return 1;
    }

    RunRepository runRepository(database);
    EventRepository eventRepository(database);
    TrendSampleRepository trendSampleRepository(database);
    PersistenceRetryQueue retryQueue(runRepository, eventRepository, trendSampleRepository);
    EventHistoryModel eventHistoryModel(database);
    RunListModel runListModel(database);
    TrendHistorySource trendHistorySource(database);
    QString runsError;

    if (!runListModel.reload(&runsError))
    {
        qCritical() << "Cannot load runs:" << runsError;
        return 1;
    }

    QString historyError;
    if (!eventHistoryModel.reload(&historyError))
    {
        qCritical() << "Cannot load event history:" << historyError;
        return 1;
    }


    Pump pump1(1); // Water supply
    Valve valve1(1);
    Pump pump2(2); // Concentrate supply
    Valve valve2(2);
    Pump pump3(3); // Finished product transfer
    Valve valve3(3);
    Valve valve4(4); // Emergency drain
    Mixer mixer;
    MixingTank mixingTank;

    pump1.setTargetRpm(ProcessConfiguration::Pump1DefaultRpm);
    pump2.setTargetRpm(ProcessConfiguration::Pump2DefaultRpm);
    pump3.setTargetRpm(ProcessConfiguration::Pump3DefaultRpm);

    EventManager eventManager;

    ProcessEventCoordinator processEventCoordinator(eventManager);

    processEventCoordinator.watchTemperature(mixingTank);
    processEventCoordinator.watchFaults(pump1, valve1, pump2, valve2, pump3, valve3, mixer, mixingTank, valve4);
    processEventCoordinator.watchMixerConnection(mixer);

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &mixingTank);
    simulator.setEnabled(!hardwareMode);

    BatchController batchController(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &valve4, &mixer, &mixingTank);
    TrendRecorder trendRecorder(&mixingTank);

    connectHistoryPersistence(eventManager, trendRecorder, retryQueue, eventHistoryModel);

    QTimer batchTimer;
    batchTimer.setInterval(100);

    QObject::connect(&batchTimer, &QTimer::timeout, &batchController, [&batchController]() {
        batchController.simulateStep(0.1);
        }
    );

    if (!hardwareMode) batchTimer.start();


    OperatorSession operatorSession(database);
    UserManagementService userManagementService(operatorSession, userRepository, userListModel, eventManager);
    DrainService drainService(operatorSession, batchController, mixingTank, valve4, eventManager);
    FaultResetService faultResetService(operatorSession, pump1, mixer, eventManager);
    RunPersistenceCoordinator runPersistenceCoordinator(eventManager, retryQueue);
    runPersistenceCoordinator.watchBatchStart(batchController, operatorSession);
    runPersistenceCoordinator.watchTrendRecording(batchController, trendRecorder);
    processEventCoordinator.watchRpmChanges(pump1, pump2, pump3, mixer, operatorSession);
    processEventCoordinator.watchStageCompletion(batchController);
    processEventCoordinator.watchBatchState(batchController, mixingTank, operatorSession);
    processEventCoordinator.watchBatchId(batchController);
    runPersistenceCoordinator.watchBatchFinish(batchController);

    ModbusControllerClient modbusControllerClient;
    RemoteProcessController remoteProcessController(&modbusControllerClient);
    RemoteProcessSynchronizer remoteProcessSynchronizer(&modbusControllerClient, &pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &valve4, &mixer, &mixingTank);

    if (hardwareMode) {
        configureHardwareMode(modbusControllerClient, remoteProcessController, drainService, operatorSession, runPersistenceCoordinator, processEventCoordinator, trendRecorder);
    }

    connectRemoteCommandFeedback(modbusControllerClient, eventManager, pump1, pump2, pump3, hardwareMode);

    QObject *activeBatchController = hardwareMode ? static_cast<QObject *>(&remoteProcessController) : static_cast<QObject *>(&batchController);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("pump1", &pump1);
    engine.rootContext()->setContextProperty("valve1", &valve1);
    engine.rootContext()->setContextProperty("pump2", &pump2);
    engine.rootContext()->setContextProperty("valve2", &valve2);
    engine.rootContext()->setContextProperty("pump3", &pump3);
    engine.rootContext()->setContextProperty("valve3", &valve3);
    engine.rootContext()->setContextProperty("valve4", &valve4);
    engine.rootContext()->setContextProperty("mixingTank", &mixingTank);
    engine.rootContext()->setContextProperty("mixer", &mixer);

    engine.rootContext()->setContextProperty("batchController", activeBatchController);
    engine.rootContext()->setContextProperty("controllerClient", &modbusControllerClient);
    engine.rootContext()->setContextProperty("hardwareMode", hardwareMode);

    engine.rootContext()->setContextProperty("operatorSession", &operatorSession);
    engine.rootContext()->setContextProperty("eventManager", &eventManager);
    engine.rootContext()->setContextProperty("trendRecorder", &trendRecorder);
    engine.rootContext()->setContextProperty("userListModel", &userListModel);
    engine.rootContext()->setContextProperty("eventHistoryModel", &eventHistoryModel);
    engine.rootContext()->setContextProperty("runListModel", &runListModel);
    engine.rootContext()->setContextProperty("trendHistorySource", &trendHistorySource);
    engine.rootContext()->setContextProperty("faultResetService", &faultResetService);
    engine.rootContext()->setContextProperty("drainService", &drainService);
    engine.rootContext()->setContextProperty("userManagementService", &userManagementService);
    engine.rootContext()->setContextProperty("persistenceRetryQueue", &retryQueue);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []()
        {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection
    );

    if (hardwareMode)
    {
        if (!modbusControllerClient.connectToController(controllerHost))
        {
            qWarning() << "Cannot start controller connection:" << modbusControllerClient.lastError();
        }
        else
        {
            qInfo() << "Connecting to process controller at" << controllerHost << "on port" << ControllerProtocol::TcpPort;
        }
    }

    engine.loadFromModule("MixingStationHMI", "HMIScreen");

    return QGuiApplication::exec();
}