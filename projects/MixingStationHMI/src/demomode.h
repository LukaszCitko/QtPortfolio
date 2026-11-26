#ifndef DEMOMODE_H
#define DEMOMODE_H

class BatchController;
class ProcessSimulator;
class QTimer;

void configureDemoMode(bool enabled, ProcessSimulator &simulator, BatchController &controller, QTimer &batchTimer);

#endif