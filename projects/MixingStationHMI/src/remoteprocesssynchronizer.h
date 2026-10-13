#ifndef REMOTEPROCESSSYNCHRONIZER_H
#define REMOTEPROCESSSYNCHRONIZER_H

#include <QObject>

class ModbusControllerClient;
class Pump;
class Valve;
class Mixer;
class MixingTank;

class RemoteProcessSynchronizer final : public QObject
{
    Q_OBJECT

public:
    explicit RemoteProcessSynchronizer(ModbusControllerClient *client, Pump *pump1, Valve *valve1, Pump *pump2, Valve *valve2, Pump *pump3, Valve *valve3, Valve *valve4, Mixer *mixer, MixingTank *tank, QObject *parent = nullptr);

private:
    void synchronizeSnapshot();
    int effectiveProcessState(int processState);

    ModbusControllerClient *m_client;
    Pump *m_pump1;
    Valve *m_valve1;
    Pump *m_pump2;
    Valve *m_valve2;
    Pump *m_pump3;
    Valve *m_valve3;
    Valve *m_valve4;
    Mixer *m_mixer;
    MixingTank *m_tank;
    int m_lastActiveProcessState;
};

#endif // REMOTEPROCESSSYNCHRONIZER_H
