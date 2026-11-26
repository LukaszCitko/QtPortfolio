#include "processequipment.h"

#include "processconfiguration.h"

ProcessEquipment::ProcessEquipment()
{
    pump1.setTargetRpm(ProcessConfiguration::Pump1DefaultRpm);
    pump2.setTargetRpm(ProcessConfiguration::Pump2DefaultRpm);
    pump3.setTargetRpm(ProcessConfiguration::Pump3DefaultRpm);
}