#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "applicationoptions.h"
#include "demomode.h"
#include "drainservice.h"
#include "eventmanager.h"
#include "faultresetservice.h"
#include "hardwaremode.h"
#include "hmiqmlcontext.h"
#include "localprocessruntime.h"
#include "operatorservices.h"
#include "processeventcoordinator.h"
#include "processequipment.h"
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

    LocalProcessRuntime localProcess(equipment);
    TrendRecorder trendRecorder(&equipment.mixingTank);

    connectHistoryPersistence(eventManager, trendRecorder, applicationDatabase->retryQueue(), applicationDatabase->eventHistoryModel());
    localProcess.configure(!hardwareMode);

    OperatorServices operatorServices(*applicationDatabase, localProcess.controller, equipment, eventManager);
    RunPersistenceCoordinator runPersistenceCoordinator(eventManager, applicationDatabase->retryQueue());

    processEventCoordinator.watchRpmChanges(equipment.pump1, equipment.pump2, equipment.pump3, equipment.mixer, operatorServices.session);
    DemoMode::connectLocalBatchObservers(localProcess.controller, equipment.mixingTank, operatorServices.session, runPersistenceCoordinator, processEventCoordinator, trendRecorder);

    RemoteProcessRuntime remoteProcess(equipment);

    if (hardwareMode) HardwareMode::configureHardwareMode(remoteProcess.client, remoteProcess.controller, operatorServices.drain, operatorServices.session, runPersistenceCoordinator, processEventCoordinator, trendRecorder);

    connectRemoteCommandFeedback(remoteProcess.client, eventManager, equipment.pump1, equipment.pump2, equipment.pump3, hardwareMode);

    QObject *activeBatchController = hardwareMode ? static_cast<QObject *>(&remoteProcess.controller) : static_cast<QObject *>(&localProcess.controller);

    QQmlApplicationEngine engine;
    HmiQml::exposeEquipmentAndHistory(*engine.rootContext(), equipment, *applicationDatabase);
    HmiQml::exposeHmiServices(*engine.rootContext(), *activeBatchController, remoteProcess.client, hardwareMode, operatorServices, eventManager, trendRecorder);
    if (hardwareMode) HardwareMode::startHardwareConnection(remoteProcess.client, controllerHost);
    HmiQml::loadHmiScreen(engine, app);

    return QGuiApplication::exec();
}