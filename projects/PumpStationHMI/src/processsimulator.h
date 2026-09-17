#ifndef PROCESSSIMULATOR_H
#define PROCESSSIMULATOR_H

#include <QObject>
#include <QTimer>

class Pump;

class ProcessSimulator : public QObject
{
    Q_OBJECT

public:
    explicit ProcessSimulator(Pump *pump, QObject *parent = nullptr);

private slots:
    void update();

private:
    Pump *m_pump;
    QTimer m_timer;

    static constexpr double RpmStep = 50.0;



};


#endif // PROCESSSIMULATOR_H
