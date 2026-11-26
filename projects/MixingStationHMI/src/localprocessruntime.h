#ifndef LOCALPROCESSRUNTIME_H
#define LOCALPROCESSRUNTIME_H

#include <QTimer>

#include "batchcontroller.h"
#include "processsimulator.h"

struct ProcessEquipment;

class LocalProcessRuntime
{
private:
    ProcessSimulator m_simulator;

public:
    explicit LocalProcessRuntime(ProcessEquipment &equipment);

    void configure(bool enabled);

    BatchController controller;

private:
    QTimer m_batchTimer;
};

#endif