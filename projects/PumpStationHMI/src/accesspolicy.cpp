#include "accesspolicy.h"

AccessPolicy::Role AccessPolicy::roleFromText(const QString &role)
{
    if (role == "OPERATOR")
        return Role::Operator;

    if (role == "TECHNICIAN")
        return Role::Technician;

    if (role == "ADMIN")
        return Role::Admin;

    return Role::None;
}

bool AccessPolicy::canControlBatch(Role role)
{
    return role == Role::Operator
           || role == Role::Technician
           || role == Role::Admin;
}

bool AccessPolicy::canResetFault(Role role)
{
    return role == Role::Technician
           || role == Role::Admin;
}

bool AccessPolicy::canManageUsers(Role role)
{
    return role == Role::Admin;
}
bool AccessPolicy::canApproveDrain(Role role)
{
    return role == Role::Technician
           || role == Role::Admin;
}