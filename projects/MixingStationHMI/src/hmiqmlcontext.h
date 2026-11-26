#ifndef HMIQMLCONTEXT_H
#define HMIQMLCONTEXT_H

class ApplicationDatabase;
class DrainService;
class EventManager;
class FaultResetService;
class ModbusControllerClient;
class OperatorSession;
class TrendRecorder;
class QCoreApplication;
class QObject;
class QQmlApplicationEngine;
class QQmlContext;
class UserManagementService;
struct ProcessEquipment;
struct OperatorServices;

namespace HmiQml {
    void loadHmiScreen(QQmlApplicationEngine &engine, QCoreApplication &app);
    void exposeHmiServices(QQmlContext &context, QObject &activeController, ModbusControllerClient &client, bool hardwareMode, OperatorServices &operators, EventManager &events, TrendRecorder &trends);
    void exposeEquipmentAndHistory(QQmlContext &context, ProcessEquipment &equipment, ApplicationDatabase &database);
} // namespace HmiQml

#endif