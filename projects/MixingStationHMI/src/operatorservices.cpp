#include "operatorservices.h"

#include "database/applicationdatabase.h"
#include "processequipment.h"

OperatorServices::OperatorServices(ApplicationDatabase &database, BatchController &controller, ProcessEquipment &equipment, EventManager &events)
    : session(database.connection()),
    userManagement(session, database.userRepository(), database.userListModel(), events),
    drain(session, controller, equipment.mixingTank, equipment.valve4, events),
    faultReset(session, equipment.pump1, equipment.mixer, events)
{
}