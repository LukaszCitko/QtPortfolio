#include <QDebug>
#include "controllerprotocol.h"
#include "hardwaremode.h"

#include "communication/modbuscontrollerclient.h"
#include "communication/remoteprocesscontroller.h"
#include "database/runpersistencecoordinator.h"
#include "drainservice.h"
#include "operatorsession.h"
#include "processeventcoordinator.h"

void configureHardwareMode(ModbusControllerClient &client, RemoteProcessController &controller, DrainService &drainService, OperatorSession &session, RunPersistenceCoordinator &runPersistence, ProcessEventCoordinator &processEvents, TrendRecorder &trends)
{
    controller.setRecoveryAuthorization([&session]() { return session.canApproveDrain(); });
    drainService.setRemoteDrainRequest([&client]() { return client.startDrain(); });

    runPersistence.watchBatchStart(controller, session);
    runPersistence.watchBatchFinish(controller);
    runPersistence.watchControllerRecovery(controller);
    runPersistence.watchTrendRecording(controller, trends);

    processEvents.watchBatchId(controller);
    processEvents.watchBatchState(controller, session);
    processEvents.watchStageCompletion(controller);
    processEvents.watchConnection(controller);
    processEvents.watchRecovery(controller, session);
    processEvents.watchCommandResults(client);
}

void startHardwareConnection(ModbusControllerClient &client, const QString &host)
{
    if (!client.connectToController(host)) {
        qWarning() << "Cannot start controller connection:" << client.lastError();
        return;
    }

    qInfo() << "Connecting to process controller at" << host << "on port" << ControllerProtocol::TcpPort;
}