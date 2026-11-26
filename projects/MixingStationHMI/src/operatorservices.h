#ifndef OPERATORSERVICES_H
#define OPERATORSERVICES_H

#include "drainservice.h"
#include "faultresetservice.h"
#include "operatorsession.h"
#include "usermanagementservice.h"

class ApplicationDatabase;
class BatchController;
class EventManager;
struct ProcessEquipment;

struct OperatorServices
{
    OperatorServices(ApplicationDatabase &database, BatchController &controller, ProcessEquipment &equipment, EventManager &events);

    OperatorSession session;
    UserManagementService userManagement;
    DrainService drain;
    FaultResetService faultReset;
};

#endif