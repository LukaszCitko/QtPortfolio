#ifndef APPLICATIONDATABASE_H
#define APPLICATIONDATABASE_H

#include <QSqlDatabase>
#include <QString>

bool prepareApplicationDatabase(QSqlDatabase &database, QString *errorMessage);

#endif
