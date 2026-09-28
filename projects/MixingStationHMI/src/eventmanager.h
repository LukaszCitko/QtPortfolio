#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QtGlobal>

class EventManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentMessage READ currentMessage NOTIFY currentEventChanged)
    Q_PROPERTY(QString currentSource READ currentSource NOTIFY currentEventChanged)
    Q_PROPERTY(Level currentLevel READ currentLevel NOTIFY currentEventChanged)
    Q_PROPERTY(QString currentLevelText READ currentLevelText NOTIFY currentEventChanged)
    Q_PROPERTY(int eventCount READ eventCount NOTIFY eventAdded)

public:
    enum class Level
    {
        Info,
        Warning,
        Alarm
    };

    Q_ENUM(Level)

    explicit EventManager(QObject *parent = nullptr);

    QString currentMessage() const;
    QString currentSource() const;
    QString currentLevelText() const;
    Level currentLevel() const;

    int eventCount() const;
    void setBatchId(const QString &batchId);

    Q_INVOKABLE QString eventBatchId(int index) const;
    Q_INVOKABLE QString eventTime(int index) const;
    Q_INVOKABLE QString eventLevel(int index) const;
    Q_INVOKABLE QString eventSource(int index) const;
    Q_INVOKABLE QString eventMessage(int index) const;

public slots:
    void addInfo(const QString &source, const QString &message);
    void addWarning(const QString &source, const QString &message);
    void addAlarm(const QString &source, const QString &message);

signals:
    void eventAdded();
    void currentEventChanged();
    void eventRecorded(const QString &batchId, qint64 timestampMs, const QString &level, const QString &source, const QString &message);

private:
    void addEvent(Level level,
                  const QString &source,
                  const QString &message);

    QString m_currentMessage;
    QString m_currentSource;
    QString m_activeBatchId;
    Level m_currentLevel = Level::Info;

    struct EventRecord
    {
        QString time;
        Level level;
        QString source;
        QString message;
        QString batchId;
    };

    QList<EventRecord> m_events;

};

#endif // EVENTMANAGER_H