#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "pump.h"
#include "processsimulator.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Pump pump;
    ProcessSimulator simulator(&pump);

    engine.rootContext()->setContextProperty("pump", &pump);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("PumpStationHMI", "Main");


    return QGuiApplication::exec();
}
