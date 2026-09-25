#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "pump.h"
#include "valve.h"
#include "mixer.h"
#include "mixingtank.h"
#include "batchcontroller.h"
#include "processsimulator.h"
#include "processcontroller.h"
#include "eventmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;


// Process equipment

    // Water supply
    Pump pump1(1);
    Valve valve1(1);

    // Concentrate supply
    Pump pump2(2);
    Valve valve2(2);

    // Finished product transfer
    Pump pump3(3);
    Valve valve3(3);

    // Emergency drain
    Valve valve4(4);

    // Mixing tank
    Mixer mixer;
    MixingTank mixingTank;

// Process services

    ProcessSimulator simulator(
        &pump1, &valve1,
        &pump2, &valve2,
        &pump3, &valve3,

        &mixingTank);

// Batch controller
    BatchController batchController(
        &pump1, &valve1,
        &pump2, &valve2,
        &pump3, &valve3,
        &valve4,
        &mixer, &mixingTank);

    EventManager eventManager;

// Temporary controller for the existing Pump 1 / Valve 1 logic.
// TODO: replace/extend by the batch controller.

    ProcessController controller(
        &pump1,
        &valve1,
        &eventManager);


// QML context properties

    engine.rootContext()->setContextProperty("pump", &pump1);
    engine.rootContext()->setContextProperty("valve1", &valve1);

    engine.rootContext()->setContextProperty("pump2", &pump2);
    engine.rootContext()->setContextProperty("valve2", &valve2);

    engine.rootContext()->setContextProperty("pump3", &pump3);
    engine.rootContext()->setContextProperty("valve3", &valve3);

    engine.rootContext()->setContextProperty("valve4", &valve4);

    engine.rootContext()->setContextProperty("mixingTank", &mixingTank);

    engine.rootContext()->setContextProperty("controller", &controller);
    engine.rootContext()->setContextProperty("batchController", &batchController);

    engine.rootContext()->setContextProperty("eventManager",&eventManager);

// QML loading

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []()
        {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule("PumpStationHMI", "Main");

    return QGuiApplication::exec();
}