#ifndef REMOTEPROCESSRUNTIME_H
#define REMOTEPROCESSRUNTIME_H

#include "modbuscontrollerclient.h"
#include "remoteprocesscontroller.h"
#include "remoteprocesssynchronizer.h"

struct ProcessEquipment;

struct RemoteProcessRuntime
{
    explicit RemoteProcessRuntime(ProcessEquipment &equipment);

    ModbusControllerClient client;
    RemoteProcessController controller;

private:
    RemoteProcessSynchronizer m_synchronizer;
};

#endif