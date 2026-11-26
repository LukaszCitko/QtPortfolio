#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

#include "applicationoptions.h"
#include "batchcontroller.h"
#include "demomode.h"
#include "drainservice.h"
#include "eventmanager.h"
#include "faultresetservice.h"
#include "hardwaremode.h"
#include "hmiqmlcontext.h"
#include "operatorservices.h"
#include "processeventcoordinator.h"
#include "processequipment.h"
#include "processsimulator.h"
#include "remoteprocesssynchronizer.h"
#include "trendrecorder.h"
#include "usermanagementservice.h"

#include "database/applicationdatabase.h"
#include "database/historypersistence.h"
#include "database/runpersistencecoordinator.h"

#include "communication/remotecommandfeedback.h"
#include "communication/remoteprocessruntime.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const QString controllerHost = readControllerHost(app);
    const bool hardwareMode = !controllerHost.isEmpty();

    QString databaseError;
    auto applicationDatabase = ApplicationDatabase::open(databaseError);
    if (!applicationDatabase) {
        qCritical() << databaseError;
        return 1;
    }

    ProcessEquipment equipment;
    EventManager eventManager;
    ProcessEventCoordinator processEventCoordinator(eventManager);
    processEventCoordinator.watchEquipment(equipment);

    ProcessSimulator simulator(&equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.mixingTank);
    BatchController batchController(&equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.valve4, &equipment.mixer, &equipment.mixingTank);
    TrendRecorder trendRecorder(&equipment.mixingTank);

    connectHistoryPersistence(eventManager, trendRecorder, applicationDatabase->retryQueue(), applicationDatabase->eventHistoryModel());

    QTimer batchTimer;
    configureDemoMode(!hardwareMode, simulator, batchController, batchTimer);

    OperatorServices operatorServices(*applicationDatabase, batchController, equipment, eventManager);

    RunPersistenceCoordinator runPersistenceCoordinator(eventManager, applicationDatabase->retryQueue());

    processEventCoordinator.watchRpmChanges(equipment.pump1, equipment.pump2, equipment.pump3, equipment.mixer, operatorServices.session);
    connectLocalBatchObservers(batchController, equipment.mixingTank, operatorServices.session, runPersistenceCoordinator, processEventCoordinator, trendRecorder);

    RemoteProcessRuntime remoteProcess(equipment);

    if (hardwareMode) configureHardwareMode(remoteProcess.client, remoteProcess.controller, operatorServices.drain, operatorServices.session, runPersistenceCoordinator, processEventCoordinator, trendRecorder);

    connectRemoteCommandFeedback(remoteProcess.client, eventManager, equipment.pump1, equipment.pump2, equipment.pump3, hardwareMode);

    QObject *activeBatchController = hardwareMode ? static_cast<QObject *>(&remoteProcess.controller) : static_cast<QObject *>(&batchController);

    QQmlApplicationEngine engine;
    exposeEquipmentAndHistory(*engine.rootContext(), equipment, *applicationDatabase);
    exposeHmiServices(*engine.rootContext(), *activeBatchController, remoteProcess.client, hardwareMode, operatorServices.session, eventManager, trendRecorder, operatorServices.faultReset, operatorServices.drain, operatorServices.userManagement);

    if (hardwareMode) startHardwareConnection(remoteProcess.client, controllerHost);

    loadHmiScreen(engine, app);

    return QGuiApplication::exec();
}