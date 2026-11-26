#ifndef HMIQMLCONTEXT_H
#define HMIQMLCONTEXT_H

class ApplicationDatabase;
class DrainService;
class EventManager;
class FaultResetService;
class ModbusControllerClient;
class OperatorSession;
class TrendRecorder;
class QObject;
class QQmlContext;
class UserManagementService;
struct ProcessEquipment;

void exposeHmiServices(QQmlContext &context, QObject &activeController, ModbusControllerClient &client, bool hardwareMode, OperatorSession &session, EventManager &events, TrendRecorder &trends, FaultResetService &faultReset, DrainService &drain, UserManagementService &users);
void exposeEquipmentAndHistory(QQmlContext &context, ProcessEquipment &equipment, ApplicationDatabase &database);

#endif