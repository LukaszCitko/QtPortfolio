#ifndef BATCHCONTROLLER_H
#define BATCHCONTROLLER_H

#include <QObject>
#include <QPointer>

    class Pump;
class Valve;
class MixingTank;
class Mixer;

class BatchController : public QObject
{
    Q_OBJECT

public:
    enum class State
    {
        Idle,
        FillingWater,
        DosingConcentrate,
        TemperatureCheck,
        Mixing,
        ReadyForTransfer,
        Transferring,
        Paused,
        Complete
    };

    Q_ENUM(State)

    explicit BatchController(
        Pump *pump1,
        Valve *valve1,
        Pump *pump2,
        Valve *valve2,
        Pump *pump3,
        Valve *valve3,
        Valve *valve4,
        Mixer *mixer,
        MixingTank *mixingTank,
        QObject *parent = nullptr);

    State state() const;
    QString stateText() const;

    Q_INVOKABLE void startBatch();
    Q_INVOKABLE void startTransfer();
    Q_INVOKABLE void emergencyDrain();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();

    void simulateStep(double elapsedSeconds);

signals:
    void stateChanged();

private slots:
    void onTankVolumeChanged();
    void onTankTemperatureChanged();

private:
    void startWaterFilling();
    void startConcentrateDosing();
    void startTemperatureCheck();
    void startMixing();
    void finishMixing();
    void finishTransfer();

    bool canStartPump1() const;
    bool canStartPump2() const;
    bool canStartPump3() const;

    static constexpr double TargetWaterVolume = 70.0;
    static constexpr double TargetConcentrateVolume = 30.0;

    static constexpr double MinMixingTemperature = 58.0;
    static constexpr double MaxMixingTemperature = 62.0;

    static constexpr double MixingDuration = 60.0;

    QPointer<Pump> m_pump1;
    QPointer<Valve> m_valve1;

    QPointer<Pump> m_pump2;
    QPointer<Valve> m_valve2;

    QPointer<Pump> m_pump3;
    QPointer<Valve> m_valve3;
    QPointer<Valve> m_valve4;

    QPointer<MixingTank> m_mixingTank;
    QPointer<Mixer> m_mixer;

    double m_mixingElapsedSeconds;
    State m_state;
    State m_previousState;
};

#endif
