#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <QObject>
#include <QString>

class EventManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentMessage READ currentMessage NOTIFY currentEventChanged)

    Q_PROPERTY(QString currentSource READ currentSource NOTIFY currentEventChanged)

    Q_PROPERTY(Level currentLevel READ currentLevel NOTIFY currentEventChanged)

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
    Level currentLevel() const;

public slots:
    void addInfo(const QString &source, const QString &message);
    void addWarning(const QString &source, const QString &message);
    void addAlarm(const QString &source, const QString &message);

signals:
    void eventAdded();
    void currentEventChanged();

private:
    void addEvent(Level level,
                  const QString &source,
                  const QString &message);

    QString m_currentMessage;
    QString m_currentSource;
    Level m_currentLevel = Level::Info;
};

#endif // EVENTMANAGER_H