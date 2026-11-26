#include "remoteprocessruntime.h"

RemoteProcessRuntime::RemoteProcessRuntime(ProcessEquipment &equipment)
    : controller(client),
    m_synchronizer(client, equipment)
{
}