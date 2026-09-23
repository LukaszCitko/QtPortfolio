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
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)

public:

    static constexpr double MaxRpm = 1800.0;

    enum class State
    {
        Stopped,
        Running,
        Fault
    };

    explicit Pump(int id, QObject *parent = nullptr);

    int id() const;
    bool isRunning() const;
    State state() const;

    double targetRpm() const;
    double actualRpm() const;
    double temperatureFromSensor() const;  // measured in Celsius
    void setActualRpm(double rpm);
    QString stateText() const;

    double maxRpm() const {return MaxRpm;}
    // extended to QML
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setFault();
    Q_INVOKABLE void resetFault();
    Q_INVOKABLE void setTargetRpm(double rpm);
    Q_INVOKABLE void setTemperatureFromSensor(double newTemperature);




private:
    int m_id;
    State m_state;
    double m_targetRpm;
    double m_actualRpm;
    double m_temperatureFromSensor;
signals:
    void targetRpmChanged();
    void actualRpmChanged();
    void temperatureFromSensorChanged();
    void stateChanged();
};

#endif // PUMP_H
