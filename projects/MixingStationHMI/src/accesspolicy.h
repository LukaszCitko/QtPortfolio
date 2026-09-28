#ifndef ACCESSPOLICY_H
#define ACCESSPOLICY_H

#include <QString>

class AccessPolicy
{
public:
    enum class Role {
        None,
        Operator,
        Technician,
        Admin
    };

    static Role roleFromText(const QString &role);

    static bool canControlBatch(Role role);
    static bool canResetFault(Role role);
    static bool canManageUsers(Role role);
    static bool canApproveDrain(Role role);
};

#endif