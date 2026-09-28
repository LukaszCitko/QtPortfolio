#ifndef DATABASESCHEMA_H
#define DATABASESCHEMA_H

#include <QSqlDatabase>
#include <QString>

bool initializeDatabaseSchema(QSqlDatabase &database, QString *errorMessage);

#endif