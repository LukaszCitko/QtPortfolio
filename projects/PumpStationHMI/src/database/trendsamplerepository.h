#ifndef TRENDSAMPLEREPOSITORY_H
#define TRENDSAMPLEREPOSITORY_H

#include <QSqlDatabase>
#include <QString>
#include <QtGlobal>

class TrendSampleRepository
{
public:
    explicit TrendSampleRepository(const QSqlDatabase &database);

    bool saveSample(const QString &runId,
                    qint64 timestampMs,
                    double volumeL,
                    double temperatureC,
                    QString *errorMessage);

private:
    QSqlDatabase m_database;
};

#endif