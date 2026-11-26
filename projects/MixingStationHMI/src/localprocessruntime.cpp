#include "localprocessruntime.h"

#include "demomode.h"
#include "processequipment.h"

LocalProcessRuntime::LocalProcessRuntime(ProcessEquipment &equipment)
    : m_simulator(&equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.mixingTank),
    controller(&equipment.pump1, &equipment.valve1, &equipment.pump2, &equipment.valve2, &equipment.pump3, &equipment.valve3, &equipment.valve4, &equipment.mixer, &equipment.mixingTank)
{
}

void LocalProcessRuntime::configure(bool enabled)
{
    configureDemoMode(enabled, m_simulator, controller, m_batchTimer);
}