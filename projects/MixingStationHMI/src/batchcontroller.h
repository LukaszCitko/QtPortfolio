#ifndef BATCHCONTROLLER_H
#define BATCHCONTROLLER_H

#include <QObject>
#include <QPointer>
#include <QString>

class Pump;
class Valve;
class MixingTank;
class Mixer;

class BatchController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)
    Q_PROPERTY(int stageIndex READ stageIndex NOTIFY stateChanged)
    Q_PROPERTY(int mixingSecondsRemaining READ mixingSecondsRemaining NOTIFY mixingTimeChanged)
    Q_PROPERTY(QString batchId READ batchId NOTIFY batchIdChanged)
    Q_PROPERTY(bool transferAllowed READ transferAllowed NOTIFY transferAllowedChanged)
    Q_PROPERTY(bool tankEmpty READ tankEmpty NOTIFY startConditionsChanged)
    Q_PROPERTY(bool devicesHealthy READ devicesHealthy NOTIFY startConditionsChanged)
    Q_PROPERTY(bool mixerReady READ mixerReady NOTIFY startConditionsChanged)
    Q_PROPERTY(bool valvesClosed READ valvesClosed NOTIFY startConditionsChanged)
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
        Draining,
        Paused,
        Complete
    };

    enum class StopReason
    {
        None,
        OperatorPause,
        MixerFault,
        Pump1Fault,
        Pump2Fault,
        Pump3Fault,
        Valve1Fault,
        Valve2Fault,
        Valve3Fault,
        DrainValveUnavailable
    };

    Q_ENUM(StopReason)
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

    Q_INVOKABLE void startTransfer();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE bool prepareNextBatch();
    Q_INVOKABLE bool tryStartBatch(const QString &operatorName);

    StopReason stopReason() const;
    QString stopReasonText() const;
    QString batchId() const;
    QString stateText() const;
    int stageIndex() const;
    int mixingSecondsRemaining() const;
    void emergencyDrain();
    void startBatch();
    void simulateStep(double elapsedSeconds);
    bool transferAllowed() const;
    bool tankEmpty() const;
    bool devicesHealthy() const;
    bool mixerReady() const;
    bool valvesClosed() const;

signals:
    void stateChanged();
    void mixingTimeChanged();
    void stageCompleted(BatchController::State completedStage);
    void batchIdChanged();
    void transferAllowedChanged();
    void startConditionsChanged();

private slots:
    void onTankVolumeChanged();
    void onTankTemperatureChanged();

    void onMixerStateChanged();
    void onPump1StateChanged();
    void onPump2StateChanged();
    void onPump3StateChanged();

private:
    void startWaterFilling();
    void startConcentrateDosing();
    void startTemperatureCheck();
    void startMixing();
    void finishMixing();
    void finishTransfer();
    void finishDrain();

    bool canStartPump1() const;
    bool canStartPump2() const;
    bool canStartPump3() const;

    static constexpr double DrainFlowPerSecond = 5.0;
    static constexpr double TargetWaterVolume = 70.0;
    static constexpr double TargetConcentrateVolume = 30.0;

    static constexpr double MinMixingTemperature = 58.0;
    static constexpr double MaxMixingTemperature = 62.0;

    static constexpr double MixingDuration = 15.0;
    static constexpr double DosingMixerRpm = 200.0;
    static constexpr double MixingMixerRpm = 800.0;

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
    StopReason m_stopReason;
    QString m_batchId;
};

#endif
