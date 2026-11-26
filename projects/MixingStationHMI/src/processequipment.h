#ifndef PROCESSEQUIPMENT_H
#define PROCESSEQUIPMENT_H

#include "mixer.h"
#include "mixingtank.h"
#include "pump.h"
#include "valve.h"

struct ProcessEquipment
{
    ProcessEquipment();

    Pump pump1{1}; // Water supply
    Valve valve1{1};
    Pump pump2{2}; // Concentrate supply
    Valve valve2{2};
    Pump pump3{3}; // Finished product transfer
    Valve valve3{3};
    Valve valve4{4}; // Emergency drain
    Mixer mixer;
    MixingTank mixingTank;
};

#endif