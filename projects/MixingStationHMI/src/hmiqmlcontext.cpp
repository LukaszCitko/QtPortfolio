#include <QQmlContext>
#include <QCoreApplication>
#include <QQmlApplicationEngine>

#include "database/applicationdatabase.h"

#include "communication/modbuscontrollerclient.h"
#include "drainservice.h"
#include "eventmanager.h"
#include "faultresetservice.h"
#include "hmiqmlcontext.h"
#include "operatorsession.h"
#include "processequipment.h"
#include "trendrecorder.h"
#include "usermanagementservice.h"

void exposeEquipmentAndHistory(QQmlContext &context, ProcessEquipment &equipment, ApplicationDatabase &database)
{
    context.setContextProperty("pump1", &equipment.pump1);
    context.setContextProperty("valve1", &equipment.valve1);
    context.setContextProperty("pump2", &equipment.pump2);
    context.setContextProperty("valve2", &equipment.valve2);
    context.setContextProperty("pump3", &equipment.pump3);
    context.setContextProperty("valve3", &equipment.valve3);
    context.setContextProperty("valve4", &equipment.valve4);
    context.setContextProperty("mixingTank", &equipment.mixingTank);
    context.setContextProperty("mixer", &equipment.mixer);

    context.setContextProperty("userListModel", &database.userListModel());
    context.setContextProperty("eventHistoryModel", &database.eventHistoryModel());
    context.setContextProperty("runListModel", &database.runListModel());
    context.setContextProperty("trendHistorySource", &database.trendHistorySource());
    context.setContextProperty("persistenceRetryQueue", &database.retryQueue());
}

void exposeHmiServices(QQmlContext &context, QObject &activeController, ModbusControllerClient &client, bool hardwareMode, OperatorSession &session, EventManager &events, TrendRecorder &trends, FaultResetService &faultReset, DrainService &drain, UserManagementService &users)
{
    context.setContextProperty("batchController", &activeController);
    context.setContextProperty("controllerClient", &client);
    context.setContextProperty("hardwareMode", hardwareMode);
    context.setContextProperty("operatorSession", &session);
    context.setContextProperty("eventManager", &events);
    context.setContextProperty("trendRecorder", &trends);
    context.setContextProperty("faultResetService", &faultReset);
    context.setContextProperty("drainService", &drain);
    context.setContextProperty("userManagementService", &users);
}

void loadHmiScreen(QQmlApplicationEngine &engine, QCoreApplication &app)
{
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("MixingStationHMI", "HMIScreen");
}