#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "pump.h"
#include "valve.h"
#include "processsimulator.h"
#include "processcontroller.h"
#include "eventmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Pump pump1(1);
    Valve valve1(1);
    ProcessSimulator simulator(&pump1);
    EventManager eventManager;
    ProcessController controller(&pump1, &valve1, &eventManager);

    engine.rootContext()->setContextProperty("pump", &pump1);
    engine.rootContext()->setContextProperty("valve1", &valve1);
    engine.rootContext()->setContextProperty("controller", &controller);
    engine.rootContext()->setContextProperty("eventManager", &eventManager);


    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("PumpStationHMI", "Main");


    return QGuiApplication::exec();
}
