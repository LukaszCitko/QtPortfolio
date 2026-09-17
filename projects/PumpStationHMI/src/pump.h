#ifndef PUMP_H
#define PUMP_H

#include <QObject>

class Pump : public QObject
{
private:
    Q_OBJECT
    Q_PROPERTY(double maxRpm READ maxRpm CONSTANT)
    Q_PROPERTY(double targetRpm
                    READ targetRpm WRITE setTargetRpm NOTIFY targetRpmChanged)
    Q_PROPERTY(double actualRpm
                    READ actualRpm NOTIFY actualRpmChanged)
    Q_PROPERTY(double temperatureFromSensor
                    READ temperatureFromSensor NOTIFY temperatureFromSensorChanged)


public:

    static constexpr double MaxRpm = 1800.0;

    enum class State
    {
        Stopped,
        Running,
        Fault
    };

    explicit Pump(QObject *parent = nullptr);

    bool isRunning() const;
    State state() const;

    double targetRpm() const;
    double actualRpm() const;
    double temperatureFromSensor() const;  // measured in Celsius
    void setActualRpm(double rpm);

    double maxRpm() const {return MaxRpm;}

    // extended to QML
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setFault();
    Q_INVOKABLE void resetFault();
    Q_INVOKABLE void setTargetRpm(double rpm);
    Q_INVOKABLE void setTemperatureFromSensor(double newTemperature);




private:
    State m_state;
    double m_targetRpm;
    double m_actualRpm;
    double m_temperatureFromSensor;
signals:
    void targetRpmChanged();
    void actualRpmChanged();
    void temperatureFromSensorChanged();
};

#endif // PUMP_H
