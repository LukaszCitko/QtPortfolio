#ifndef PROCESSCONTROLLER_H
#define PROCESSCONTROLLER_H

#include <QObject>
#include <QPointer>

class Pump;
class Valve;
class EventManager;

class ProcessController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString startBlockedReason READ startBlockedReason NOTIFY startBlockedReasonChanged)

public:
    explicit ProcessController(Pump *pump,
                               Valve *valve,
                               EventManager *eventManager,
                               QObject *parent = nullptr);


signals:
    void startBlockedReasonChanged();

public slots:
    void startPump();
    void stopPump();
    Q_INVOKABLE void setPumpFault();
    Q_INVOKABLE void resetPumpFault();

private slots:
    void onValveStateChanged();
    void onTemperatureChanged();

private:
    QPointer<Pump> m_pump;
    QPointer<Valve> m_valve;
    QPointer<EventManager> m_eventManager;
    QString m_startBlockedReason;
    QString startBlockedReason() const;
    static constexpr double MaxTemperature = 90.0;

    void updateStartBlockedReason();
};

#endif // PROCESSCONTROLLER_H