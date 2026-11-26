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

#include "operatorsession.h"

#include "processeventcoordinator.h"
#include "processequipment.h"
#include "processsimulator.h"

#include "remoteprocesssynchronizer.h"
#include "trendrecorder.h"
#include "usermanagementservice.h"

#include "database/applicationdatabase.h"
#include "database/historypersistence.h"
#include "database/runpersistencecoordinator.h"

#include "communication/modbuscontrollerclient.h"
#include "communication/remoteprocesscontroller.h"
#include "communication/remotecommandfeedback.h"

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

    processEventCoordinator.watchTemperature(equipment.mixingTank);
    processEventCoordinator.watchFaults(equipment.pump1, equipment.valve1, equipment.pump2, equipment.valve2, equipment.pump3, equipment.valve3, equipment.mixer, equipment.mixingTank, equipment.valve4);
    processEventCoordinator.watchMixerConnection(equipment.mixer);

    ProcessSimulator simulator(&equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.mixingTank);
    BatchController batchController(&equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.valve4, &equipment.mixer, &equipment.mixingTank);
    TrendRecorder trendRecorder(&equipment.mixingTank);

    connectHistoryPersistence(eventManager, trendRecorder, applicationDatabase->retryQueue(), applicationDatabase->eventHistoryModel());

    QTimer batchTimer;
    configureDemoMode(!hardwareMode, simulator, batchController, batchTimer);

    OperatorSession operatorSession(applicationDatabase->connection());
    UserManagementService userManagementService(operatorSession, applicationDatabase->userRepository(), applicationDatabase->userListModel(), eventManager);
    DrainService drainService(operatorSession, batchController, equipment.mixingTank, equipment.valve4, eventManager);
    FaultResetService faultResetService(operatorSession, equipment.pump1, equipment.mixer, eventManager);
    RunPersistenceCoordinator runPersistenceCoordinator(eventManager, applicationDatabase->retryQueue());

    processEventCoordinator.watchRpmChanges(equipment.pump1, equipment.pump2, equipment.pump3, equipment.mixer, operatorSession);
    connectLocalBatchObservers(batchController, equipment.mixingTank, operatorSession, runPersistenceCoordinator, processEventCoordinator, trendRecorder);

    ModbusControllerClient modbusControllerClient;
    RemoteProcessController remoteProcessController(&modbusControllerClient);
    RemoteProcessSynchronizer remoteProcessSynchronizer(&modbusControllerClient, &equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.valve4, &equipment.mixer, &equipment.mixingTank);

    if (hardwareMode) configureHardwareMode(modbusControllerClient, remoteProcessController, drainService, operatorSession, runPersistenceCoordinator, processEventCoordinator, trendRecorder);

    connectRemoteCommandFeedback(modbusControllerClient, eventManager, equipment.pump1, equipment.pump2, equipment.pump3, hardwareMode);

    QObject *activeBatchController = hardwareMode ? static_cast<QObject *>(&remoteProcessController) : static_cast<QObject *>(&batchController);

    QQmlApplicationEngine engine;
    exposeEquipmentAndHistory(*engine.rootContext(), equipment, *applicationDatabase);
    exposeHmiServices(*engine.rootContext(), *activeBatchController, modbusControllerClient, hardwareMode, operatorSession, eventManager, trendRecorder, faultResetService, drainService, userManagementService);

    if (hardwareMode) startHardwareConnection(modbusControllerClient, controllerHost);

    loadHmiScreen(engine, app);

    return QGuiApplication::exec();
}