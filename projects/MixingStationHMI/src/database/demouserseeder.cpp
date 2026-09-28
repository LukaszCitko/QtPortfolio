#include "demouserseeder.h"

#include <QSqlError>
#include <QSqlQuery>

bool seedDemoUsersIfEmpty(const QSqlDatabase &database,
                          QString *errorMessage)
{
    QSqlQuery existing(database);

    if (!existing.exec("SELECT 1 FROM users LIMIT 1")) {
        *errorMessage = existing.lastError().text();
        return false;
    }

    if (existing.next())
        return true;

    QSqlQuery seed(database);

    if (!seed.exec(
            "INSERT INTO users (user_id, display_name, role) VALUES "
            "('demo-operator-1', 'Demo Operator 1', 'OPERATOR'), "
            "('demo-operator-2', 'Demo Operator 2', 'OPERATOR'), "
            "('demo-technician', 'Demo Technician', 'TECHNICIAN'), "
            "('demo-admin', 'Demo Admin', 'ADMIN')")) {
        *errorMessage = seed.lastError().text();
        return false;
    }

    return true;
}