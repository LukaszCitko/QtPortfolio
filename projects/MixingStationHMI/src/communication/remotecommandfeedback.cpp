#include "remotecommandfeedback.h"

#include "modbuscontrollerclient.h"
#include "eventmanager.h"
#include "pump.h"

#include <QObject>
#include <QTimer>

void connectRemoteCommandFeedback(ModbusControllerClient &client, EventManager &events, Pump &pump1, Pump &pump2, Pump &pump3, bool hardwareMode)
{
    ModbusControllerClient *clientPtr = &client;
    EventManager *eventsPtr = &events;
    Pump *pump1Ptr = &pump1;
    Pump *pump2Ptr = &pump2;
    Pump *pump3Ptr = &pump3;

    QObject::connect(clientPtr, &ModbusControllerClient::pumpTargetRpmWriteFinished, eventsPtr, [clientPtr, eventsPtr, pump1Ptr, pump2Ptr, pump3Ptr](int pumpNumber, int requestedRpm, bool success) {
        Pump *pump = nullptr;

        if (pumpNumber == 1)
            pump = pump1Ptr;
        else if (pumpNumber == 2)
            pump = pump2Ptr;
        else if (pumpNumber == 3)
            pump = pump3Ptr;

        if (pump == nullptr) {
            eventsPtr->addWarning("COMMUNICATION", "RPM confirmation contains an unknown pump number");
            return;
        }

        if (!success) {
            eventsPtr->addWarning(QStringLiteral("P%1").arg(pumpNumber), "Target RPM write failed: " + clientPtr->lastError());
            return;
        }

        pump->setTargetRpm(static_cast<double>(requestedRpm));
    });

    QObject::connect(clientPtr, &ModbusControllerClient::simulatedFaultMaskWriteFinished, clientPtr, [clientPtr, hardwareMode](quint16 faultMask, bool success) {
        if (!hardwareMode || !success || faultMask != 0U)
            return;

        QTimer::singleShot(250, clientPtr, [clientPtr]() {
            if (!clientPtr->commandPending())
                clientPtr->resetFaults();
        });
    });
}