#ifndef PROCESSEVENTCOORDINATOR_H
#define PROCESSEVENTCOORDINATOR_H

class EventManager;
class MixingTank;
class Pump;
class Valve;
class Mixer;
class OperatorSession;
class BatchController;
class RemoteProcessController;

class ProcessEventCoordinator
{
public:
    explicit ProcessEventCoordinator(EventManager &events);

    void watchMixerConnection(Mixer &mixer);
    void watchTemperature(MixingTank &tank);
    void watchFaults(Pump &pump1, Valve &valve1, Pump &pump2, Valve &valve2, Pump &pump3, Valve &valve3, Mixer &mixer, MixingTank &tank, Valve &valve4);
    void watchRpmChanges(Pump &pump1, Pump &pump2, Pump &pump3, Mixer &mixer, OperatorSession &session);

    void watchBatchState(BatchController &controller, MixingTank &tank, OperatorSession &session);
    void watchBatchId(BatchController &controller);
    void watchStageCompletion(BatchController &controller);

    void watchBatchState(RemoteProcessController &controller, OperatorSession &session);
    void watchBatchId(RemoteProcessController &controller);
    void watchStageCompletion(RemoteProcessController &controller);
    void watchConnection(RemoteProcessController &controller);
    void watchRecovery(RemoteProcessController &controller, OperatorSession &session);
private:
    EventManager &m_events;
};

#endif