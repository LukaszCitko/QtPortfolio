#ifndef TRENDHISTORYSOURCE_H
#define TRENDHISTORYSOURCE_H

#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class TrendHistorySource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY samplesChanged)
    Q_PROPERTY(QString runId READ runId NOTIFY runChanged)

public:
    explicit TrendHistorySource(const QSqlDatabase &database,
                                QObject *parent = nullptr);

    int sampleCount() const;
    QString runId() const;

    Q_INVOKABLE QString runIdAt(int index) const;
    Q_INVOKABLE qint64 timestampMsAt(int index) const;
    Q_INVOKABLE double volumeAt(int index) const;
    Q_INVOKABLE double temperatureAt(int index) const;

    Q_INVOKABLE bool loadRun(const QString &runId);

signals:
    void samplesChanged();
    void runChanged();

private:
    struct Sample {
        qint64 timestampMs;
        double volumeL;
        double temperatureC;
    };

    QSqlDatabase m_database;
    QList<Sample> m_samples;
    QString m_runId;
};

#endif