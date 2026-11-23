#ifndef REMOTECOMMANDFEEDBACK_H
#define REMOTECOMMANDFEEDBACK_H

class ModbusControllerClient;
class EventManager;
class Pump;

void connectRemoteCommandFeedback(ModbusControllerClient &client, EventManager &events, Pump &pump1, Pump &pump2, Pump &pump3, bool hardwareMode);

#endif