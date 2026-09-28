#ifndef DEMOUSERSEEDER_H
#define DEMOUSERSEEDER_H

#include <QSqlDatabase>
#include <QString>

bool seedDemoUsersIfEmpty(const QSqlDatabase &database,
                          QString *errorMessage);

#endif