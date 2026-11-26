#ifndef HARDWAREMODE_H
#define HARDWAREMODE_H

#include <QString>

class DrainService;
class ModbusControllerClient;
class OperatorSession;
class ProcessEventCoordinator;
class RemoteProcessController;
class RunPersistenceCoordinator;
class TrendRecorder;

void configureHardwareMode(ModbusControllerClient &client, RemoteProcessController &controller, DrainService &drainService, OperatorSession &session, RunPersistenceCoordinator &runPersistence, ProcessEventCoordinator &processEvents, TrendRecorder &trends);
void startHardwareConnection(ModbusControllerClient &client, const QString &host);

#endif