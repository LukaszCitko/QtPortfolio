#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTimer>


#include "batchcontroller.h"
#include "drainservice.h"
#include "eventmanager.h"
#include "faultresetservice.h"
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

#include "database/databaseschema.h"
#include "database/demouserseeder.h"
#include "database/eventhistorymodel.h"
#include "database/eventrepository.h"
#include "database/runlistmodel.h"
#include "database/runpersistencecoordinator.h"
#include "database/runrepository.h"
#include "database/trendhistorysource.h"
#include "database/trendsamplerepository.h"
#include "database/userlistmodel.h"
#include "database/userrepository.h"

#include "communication/modbuscontrollerclient.h"
#include "communication/remoteprocesscontroller.h"

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


    const QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (dataPath.isEmpty() || !QDir().mkpath(dataPath))
    {
        qCritical() << "Cannot create application data directory:" << dataPath;
        return 1;
    }

    // Preserve data created under the previous application name.
    const QString databasePath =
        QDir(dataPath).filePath("mixingstation.sqlite");

    const QString legacyDatabasePath =
        QDir(dataPath).filePath("pumpstation.sqlite");

    if (!QFile::exists(databasePath)
        && QFile::exists(legacyDatabasePath))
    {
        if (!QFile::rename(legacyDatabasePath, databasePath))
        {
            qCritical() << "Cannot rename legacy database:"
                        << legacyDatabasePath
                        << "to"
                        << databasePath;
            return 1;
        }

        qInfo() << "Legacy database renamed to" << databasePath;
    }

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(databasePath);

    if (!database.open())
    {
        qCritical() << "Cannot open SQLite database:"
                    << database.lastError().text();
        return 1;
    }

    QSqlQuery foreignKeys(database);
    if (!foreignKeys.exec("PRAGMA foreign_keys = ON"))
    {
        qCritical() << "Cannot enable SQLite foreign keys:"
                    << foreignKeys.lastError().text();
        return 1;
    }

    QString schemaError;
    if (!initializeDatabaseSchema(database, &schemaError))
    {
        qCritical() << schemaError;
        return 1;
    }
    qInfo() << "SQLite database opened at" << database.databaseName();

    QString demoUsersError;

    if (!seedDemoUsersIfEmpty(database, &demoUsersError))
    {
        qCritical() << "Cannot prepare demo users:" << demoUsersError;
        return 1;
    }

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

    QObject::connect(&eventManager, &EventManager::eventRecorded, &eventManager,
                     [&eventRepository, &eventHistoryModel](
                         const QString &runId, qint64 timestampMs,
                         const QString &level, const QString &source,
                         const QString &message)
                     {
                         QString error;

                         if (!eventRepository.saveEvent(runId, timestampMs, level,
                                                        source, message, &error))
                         {
                             qCritical() << "Cannot save event:" << error;
                             return;
                         }

                         eventHistoryModel.prependSavedEvent(
                             runId, timestampMs, level, source, message);
                     }
    );

    ProcessEventCoordinator processEventCoordinator(eventManager);

    processEventCoordinator.watchTemperature(mixingTank);
    processEventCoordinator.watchFaults(pump1, valve1, pump2, valve2, pump3, valve3, mixer, mixingTank, valve4);
    processEventCoordinator.watchMixerConnection(mixer);

    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &mixingTank);
    simulator.setEnabled(!hardwareMode);

    BatchController batchController(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &valve4, &mixer, &mixingTank);
    TrendRecorder trendRecorder(&mixingTank);

    QObject::connect(&trendRecorder, &TrendRecorder::sampleRecorded, &trendRecorder,
                     [&trendSampleRepository](const QString &runId, qint64 timestampMs, double volumeL, double temperatureC)
                     {
                         QString error;
                         if (!trendSampleRepository.saveSample(runId, timestampMs, volumeL, temperatureC, &error))
                         {
                             qCritical() << "Cannot save trend sample:" << error;
                         }
                     }
    );

    QTimer batchTimer;
    batchTimer.setInterval(100);

    QObject::connect(&batchTimer, &QTimer::timeout, &batchController,
                     [&batchController]()
                     {
                         batchController.simulateStep(0.1);
                     }
    );

    if (!hardwareMode)
    {
        batchTimer.start();
    }

    OperatorSession operatorSession(database);
    UserManagementService userManagementService(operatorSession, userRepository, userListModel, eventManager);
    DrainService drainService(operatorSession, batchController, mixingTank, valve4, eventManager);
    FaultResetService faultResetService(operatorSession, pump1, mixer, eventManager);
    RunPersistenceCoordinator runPersistenceCoordinator(runRepository, eventManager);

    runPersistenceCoordinator.watchBatchStart(batchController, operatorSession);
    runPersistenceCoordinator.watchTrendRecording(batchController, trendRecorder);
    processEventCoordinator.watchRpmChanges(pump1, pump2, pump3, mixer, operatorSession);
    processEventCoordinator.watchStageCompletion(batchController);
    processEventCoordinator.watchBatchState(batchController, mixingTank, operatorSession);
    processEventCoordinator.watchBatchId(batchController);
    runPersistenceCoordinator.watchBatchFinish(batchController);

    ModbusControllerClient modbusControllerClient;
    RemoteProcessController remoteProcessController(&modbusControllerClient);
    remoteProcessController.setRecoveryAuthorization([&operatorSession]()
    {
        return operatorSession.canApproveDrain();
    }
    );

    RemoteProcessSynchronizer remoteProcessSynchronizer(&modbusControllerClient, &pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &valve4, &mixer, &mixingTank);




    if (hardwareMode)
    {
        drainService.setRemoteDrainRequest([&modbusControllerClient]()
                                           {
                                               return modbusControllerClient.startDrain();
                                           });

        runPersistenceCoordinator.watchBatchStart(remoteProcessController, operatorSession);
        runPersistenceCoordinator.watchBatchFinish(remoteProcessController);
        runPersistenceCoordinator.watchControllerRecovery(remoteProcessController);
        runPersistenceCoordinator.watchTrendRecording(remoteProcessController, trendRecorder);
        processEventCoordinator.watchBatchId(remoteProcessController);
        processEventCoordinator.watchBatchState(remoteProcessController, operatorSession);
        processEventCoordinator.watchStageCompletion(remoteProcessController);
        processEventCoordinator.watchConnection(remoteProcessController);
        processEventCoordinator.watchRecovery(remoteProcessController, operatorSession);
    }

    QObject::connect(&modbusControllerClient, &ModbusControllerClient::pumpTargetRpmWriteFinished, &eventManager, [&pump1, &pump2, &pump3, &eventManager, &modbusControllerClient](int pumpNumber, int requestedRpm, bool success)
                    {
                        Pump *pump = nullptr;

                        if (pumpNumber == 1)
                        {
                            pump = &pump1;
                        }
                        else if (pumpNumber == 2)
                        {
                            pump = &pump2;
                        }
                        else if (pumpNumber == 3)
                        {
                            pump = &pump3;
                        }

                        if (pump == nullptr)
                        {
                            eventManager.addWarning("COMMUNICATION", "RPM confirmation contains an unknown pump number");
                            return;
                        }

                        if (!success)
                        {
                            eventManager.addWarning(QString("P%1").arg(pumpNumber), "Target RPM write failed: " + modbusControllerClient.lastError());
                            return;
                        }

                        pump->setTargetRpm(static_cast<double>(requestedRpm));
                    }
    );

    QObject::connect(&modbusControllerClient, &ModbusControllerClient::simulatedFaultMaskWriteFinished, &modbusControllerClient, [&modbusControllerClient, hardwareMode](quint16 faultMask, bool success)
                     {
                        if (!hardwareMode || !success || faultMask != 0U)
                        {
                            return;
                        }

                        QTimer::singleShot(250, &modbusControllerClient, [&modbusControllerClient]()
                                            {
                                                if (!modbusControllerClient.commandPending())
                                                {
                                                    modbusControllerClient.resetFaults();
                                                }
                                            }
                        );
                     }
    );
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