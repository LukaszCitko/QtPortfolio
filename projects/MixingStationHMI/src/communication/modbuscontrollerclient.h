#ifndef MODBUSCONTROLLERCLIENT_H
#define MODBUSCONTROLLERCLIENT_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtGlobal>

#include <QModbusDataUnit>
#include <QModbusDevice>
#include <QModbusTcpClient>

class ModbusControllerClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(bool snapshotValid READ snapshotValid NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 protocolVersion READ protocolVersion NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 heartbeat READ heartbeat NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 controllerSessionId READ controllerSessionId NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 runSequence READ runSequence NOTIFY snapshotChanged)
    Q_PROPERTY(int runKind READ runKind NOTIFY snapshotChanged)
    Q_PROPERTY(int processState READ processState NOTIFY snapshotChanged)
    Q_PROPERTY(double stageProgress READ stageProgress NOTIFY snapshotChanged)
    Q_PROPERTY(double mixtureVolumeLitres READ mixtureVolumeLitres NOTIFY snapshotChanged)
    Q_PROPERTY(double temperatureCelsius READ temperatureCelsius NOTIFY snapshotChanged)
    Q_PROPERTY(double waterVolumeLitres READ waterVolumeLitres NOTIFY snapshotChanged)
    Q_PROPERTY(double concentrateVolumeLitres READ concentrateVolumeLitres NOTIFY snapshotChanged)
    Q_PROPERTY(int pump1ActualRpm READ pump1ActualRpm NOTIFY snapshotChanged)
    Q_PROPERTY(int pump2ActualRpm READ pump2ActualRpm NOTIFY snapshotChanged)
    Q_PROPERTY(int pump3ActualRpm READ pump3ActualRpm NOTIFY snapshotChanged)
    Q_PROPERTY(int mixerActualRpm READ mixerActualRpm NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 equipmentStatusMask READ equipmentStatusMask NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 faultMask READ faultMask NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 lastHandledCommandSequence READ lastHandledCommandSequence NOTIFY snapshotChanged)
    Q_PROPERTY(quint16 simulatedFaultMask READ simulatedFaultMask NOTIFY simulatedFaultMaskChanged)
    Q_PROPERTY(int lastCommandResult READ lastCommandResult NOTIFY snapshotChanged)
    Q_PROPERTY(bool commandPending READ commandPending NOTIFY commandPendingChanged)
    Q_PROPERTY(bool temperatureSensorValid READ temperatureSensorValid NOTIFY snapshotChanged)

public:
    explicit ModbusControllerClient(QObject *parent = nullptr);

    bool isConnected() const;
    QString lastError() const;

    bool snapshotValid() const;
    quint16 protocolVersion() const;
    quint16 heartbeat() const;
    quint16 controllerSessionId() const;
    quint16 runSequence() const;
    int runKind() const;
    int processState() const;
    double stageProgress() const;
    double mixtureVolumeLitres() const;
    double waterVolumeLitres() const;
    double concentrateVolumeLitres() const;
    double temperatureCelsius() const;

    int pump1ActualRpm() const;
    int pump2ActualRpm() const;
    int pump3ActualRpm() const;
    int mixerActualRpm() const;

    quint16 equipmentStatusMask() const;
    quint16 faultMask() const;
    quint16 simulatedFaultMask() const;

    quint16 lastHandledCommandSequence() const;
    int lastCommandResult() const;
    bool commandPending() const;
    bool temperatureSensorValid() const;

    Q_INVOKABLE bool connectToController(const QString &host);
    Q_INVOKABLE void disconnectFromController();
    Q_INVOKABLE bool startBatch();
    Q_INVOKABLE bool pause();
    Q_INVOKABLE bool resume();
    Q_INVOKABLE bool startTransfer();
    Q_INVOKABLE bool resetFaults();
    Q_INVOKABLE bool startDrain();
    Q_INVOKABLE bool prepareNextBatch();
    Q_INVOKABLE bool setPumpTargetRpm(int pumpNumber, int rpm);
    Q_INVOKABLE bool setSimulatedFaultMask(quint16 faultMask);
    Q_INVOKABLE bool setSimulatedFault(quint16 faultBit, bool active);

signals:
    void connectionChanged();
    void lastErrorChanged();
    void snapshotChanged();
    void commandPendingChanged();
    void pumpTargetRpmWriteFinished(int pumpNumber, int requestedRpm, bool success);
    void simulatedFaultMaskWriteFinished(quint16 faultMask, bool success);
    void simulatedFaultMaskChanged();
    void commandFinished(quint16 commandCode, quint16 commandSequence, int result);

private:
    bool sendCommand(quint16 commandCode);
    void requestSnapshot();
    void processSnapshot(const QModbusDataUnit &registers);

    void setLastError(const QString &message);

    QModbusTcpClient m_client;
    QTimer m_pollTimer;
    QTimer m_reconnectTimer;

    QString m_controllerHost;
    QString m_lastError;

    bool m_disconnectRequested = false;
    bool m_readPending = false;
    bool m_snapshotValid = false;

    quint16 m_protocolVersion = 0;
    quint16 m_heartbeat = 0;
    quint16 m_controllerSessionId = 0;
    quint16 m_runSequence = 0;
    int m_runKind = 0;
    int m_processState = 0;
    double m_stageProgress = 0.0;

    double m_mixtureVolumeLitres = 0.0;
    double m_waterVolumeLitres = 0.0;
    double m_concentrateVolumeLitres = 0.0;
    double m_temperatureCelsius = 0.0;

    int m_pump1ActualRpm = 0;
    int m_pump2ActualRpm = 0;
    int m_pump3ActualRpm = 0;
    int m_mixerActualRpm = 0;

    quint16 m_equipmentStatusMask = 0;
    quint16 m_faultMask = 0;

    int m_lastCommandResult = 0;
    bool m_commandPending = false;
    bool writePumpTargetRpm(int pumpNumber, quint16 registerAddress, int rpm);
    quint16 m_lastHandledCommandSequence = 0;
    quint16 m_nextCommandSequence = 1;
    quint16 m_pendingCommandCode = 0;
    quint16 m_pendingCommandSequence = 0;
    quint16 m_simulatedFaultMask = 0;
};

#endif // MODBUSCONTROLLERCLIENT_H