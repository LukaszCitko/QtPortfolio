
#ifndef PROCESSSIMULATOR_H
#define PROCESSSIMULATOR_H

#include <QObject>
#include <QPointer>
#include <QTimer>

    class Pump;
class Valve;
class MixingTank;

class ProcessSimulator : public QObject
{
    Q_OBJECT

public:
    explicit ProcessSimulator(
        Pump *pump1,
        Valve *valve1,
        Pump *pump2,
        Valve *valve2,
        Pump *pump3,
        Valve *valve3,

        MixingTank *mixingTank,
        QObject *parent = nullptr);

    void simulateStep(double elapsedSeconds);

private slots:
    void updateSimulation();

private:
    QPointer<Pump> m_pump1;
    QPointer<Valve> m_valve1;

    QPointer<Pump> m_pump2;
    QPointer<Valve> m_valve2;

    QPointer<Pump> m_pump3;
    QPointer<Valve> m_valve3;

    QPointer<MixingTank> m_mixingTank;

    QTimer m_timer;

    static constexpr double RpmStep = 50.0;
    static constexpr double WaterFlowPerSecond = 1.0;
    static constexpr double ConcentrateFlowPerSecond = 1.0;
    static constexpr double ProductFlowPerSecond = 2.0;
};

#endif
