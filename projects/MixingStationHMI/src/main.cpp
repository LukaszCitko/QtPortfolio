#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>


#include "pump.h"
#include "valve.h"
#include "mixer.h"
#include "mixingtank.h"
#include "batchcontroller.h"
#include "processsimulator.h"
#include "eventmanager.h"
#include "operatorsession.h"
#include "trendrecorder.h"
#include "drainservice.h"

#include "faultresetservice.h"

#include "database/databaseschema.h"
#include "database/userlistmodel.h"
#include "database/runrepository.h"
#include "processeventcoordinator.h"
#include "database/runpersistencecoordinator.h"
#include "database/eventrepository.h"
#include "database/eventhistorymodel.h"
#include "database/trendsamplerepository.h"
#include "database/runlistmodel.h"
#include "database/trendhistorysource.h"
#include "database/userrepository.h"
#include "database/demouserseeder.h"
#include "usermanagementservice.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (dataPath.isEmpty() || !QDir().mkpath(dataPath)) {
        qCritical() << "Cannot create application data directory:" << dataPath;
        return 1;
    }

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(QDir(dataPath).filePath("pumpstation.sqlite"));

    if (!database.open()) {qCritical() << "Cannot open SQLite database:"
                    << database.lastError().text();
        return 1;
    }

    QSqlQuery foreignKeys(database);
    if (!foreignKeys.exec("PRAGMA foreign_keys = ON")) {
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

    if (!seedDemoUsersIfEmpty(database, &demoUsersError)) {
        qCritical() << "Cannot prepare demo users:" << demoUsersError;
        return 1;
    }

    UserListModel userListModel(database);
    UserRepository userRepository(database);

    QString usersError;
    if (!userListModel.reload(&usersError)) {
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
    if (!runListModel.reload(&runsError)) {
        qCritical() << "Cannot load runs:" << runsError;
        return 1;
    }

    QString historyError;
    if (!eventHistoryModel.reload(&historyError)) {
        qCritical() << "Cannot load event history:" << historyError;
        return 1;
    }

    QQmlApplicationEngine engine;


    Pump pump1(1); // Water supply
    Valve valve1(1);
    Pump pump2(2); // Concentrate supply
    Valve valve2(2);
    Pump pump3(3); // Finished product transfer
    Valve valve3(3);
    Valve valve4(4); // Emergency drain
    Mixer mixer;
    MixingTank mixingTank;

    EventManager eventManager;

    QObject::connect(&eventManager, &EventManager::eventRecorded, &eventManager,
                     [&eventRepository, &eventHistoryModel](
                         const QString &runId, qint64 timestampMs,
                         const QString &level, const QString &source,
                         const QString &message)
                     {
                         QString error;

                         if (!eventRepository.saveEvent(runId, timestampMs, level,
                                                        source, message, &error)) {
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

// Process services
    ProcessSimulator simulator(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &mixingTank);

// Batch controller
    BatchController batchController(&pump1, &valve1, &pump2, &valve2, &pump3, &valve3, &valve4, &mixer, &mixingTank);

//Trend recorder
    TrendRecorder trendRecorder(&mixingTank);

    QObject::connect(&trendRecorder, &TrendRecorder::sampleRecorded, &trendRecorder,
                     [&trendSampleRepository](const QString &runId, qint64 timestampMs, double volumeL, double temperatureC)
                     {
                         QString error;
                         if (!trendSampleRepository.saveSample(runId, timestampMs, volumeL, temperatureC, &error)) {
                             qCritical() << "Cannot save trend sample:" << error;
                         }
                     }
    );

    QTimer batchTimer;
    batchTimer.setInterval(100);

    QObject::connect(&batchTimer, &QTimer::timeout, &batchController, [&batchController](){ batchController.simulateStep(0.1); });

    batchTimer.start();


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

// QML context properties

    engine.rootContext()->setContextProperty("pump1", &pump1);
    engine.rootContext()->setContextProperty("valve1", &valve1);
    engine.rootContext()->setContextProperty("pump2", &pump2);
    engine.rootContext()->setContextProperty("valve2", &valve2);
    engine.rootContext()->setContextProperty("pump3", &pump3);
    engine.rootContext()->setContextProperty("valve3", &valve3);
    engine.rootContext()->setContextProperty("valve4", &valve4);
    engine.rootContext()->setContextProperty("mixingTank", &mixingTank);
    engine.rootContext()->setContextProperty("mixer", &mixer);
    engine.rootContext()->setContextProperty("batchController", &batchController);
    engine.rootContext()->setContextProperty("operatorSession", &operatorSession);
    engine.rootContext()->setContextProperty("eventManager",&eventManager);
    engine.rootContext()->setContextProperty("trendRecorder", &trendRecorder);
    engine.rootContext()->setContextProperty("userListModel", &userListModel);
    engine.rootContext()->setContextProperty("eventHistoryModel", &eventHistoryModel);
    engine.rootContext()->setContextProperty("runListModel", &runListModel);
    engine.rootContext()->setContextProperty("trendHistorySource", &trendHistorySource);
    engine.rootContext()->setContextProperty("faultResetService",  &faultResetService);
    engine.rootContext()->setContextProperty("drainService", &drainService);
    engine.rootContext()->setContextProperty("userManagementService", &userManagementService);

    // QML loading

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [](){QCoreApplication::exit(-1);},Qt::QueuedConnection);

    engine.loadFromModule("MixingStationHMI", "HMIScreen");

    return QGuiApplication::exec();
}