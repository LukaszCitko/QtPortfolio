#ifndef REMOTEPROCESSCONTROLLER_H
#define REMOTEPROCESSCONTROLLER_H

#include <QObject>
#include <QString>

class ModbusControllerClient;

class RemoteProcessController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY connectionChanged)
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)
    Q_PROPERTY(int stageIndex READ stageIndex NOTIFY stateChanged)
    Q_PROPERTY(int mixingSecondsRemaining READ mixingSecondsRemaining NOTIFY mixingTimeChanged)
    Q_PROPERTY(QString batchId READ batchId NOTIFY batchIdChanged)
    Q_PROPERTY(int runKind READ runKind NOTIFY batchIdChanged)
    Q_PROPERTY(bool transferAllowed READ transferAllowed NOTIFY transferAllowedChanged)
    Q_PROPERTY(bool tankEmpty READ tankEmpty NOTIFY startConditionsChanged)
    Q_PROPERTY(bool devicesHealthy READ devicesHealthy NOTIFY startConditionsChanged)
    Q_PROPERTY(bool mixerReady READ mixerReady NOTIFY startConditionsChanged)
    Q_PROPERTY(bool valvesClosed READ valvesClosed NOTIFY startConditionsChanged)
    Q_PROPERTY(bool recoveryRequired READ recoveryRequired NOTIFY recoveryRequiredChanged)
    Q_PROPERTY(bool temperatureSensorReady READ temperatureSensorReady NOTIFY startConditionsChanged)

public:
    explicit RemoteProcessController(ModbusControllerClient *client, QObject *parent = nullptr);

    bool connected() const;
    int state() const;
    QString stateText() const;
    int stageIndex() const;
    int mixingSecondsRemaining() const;
    QString batchId() const;
    int runKind() const;

    bool temperatureSensorReady() const;
    bool transferAllowed() const;
    bool tankEmpty() const;
    bool devicesHealthy() const;
    bool mixerReady() const;
    bool valvesClosed() const;
    bool recoveryRequired() const;
    Q_INVOKABLE bool completeMechanicalRecovery();
    Q_INVOKABLE bool tryStartBatch(const QString &operatorName);
    Q_INVOKABLE bool pause();
    Q_INVOKABLE bool resume();
    Q_INVOKABLE bool startTransfer();
    Q_INVOKABLE bool prepareNextBatch();

signals:
    void connectionChanged();
    void stateChanged();
    void mixingTimeChanged();
    void batchIdChanged();
    void transferAllowedChanged();
    void startConditionsChanged();
    void connectionRestored();
    void initialSnapshotReceived();
    void recoveryRequiredChanged();

private:
    void handleSnapshotChanged();
    void handleConnectionChanged();
    static int stageIndexForState(int processState);

    ModbusControllerClient *m_client;
    QString m_batchId;
    int m_previousState;
    int m_lastActiveStageIndex;
    quint16 m_lastControllerSessionId{0};
    bool m_recoveryRequired{false};
    bool m_connectedOnce{false};
    bool m_connectionInterrupted{false};
    bool m_restoredSnapshotPending{false};
    bool m_firstSnapshotReceived{false};
};

#endif // REMOTEPROCESSCONTROLLER_H
