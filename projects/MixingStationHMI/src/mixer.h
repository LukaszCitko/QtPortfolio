#ifndef MIXER_H
#define MIXER_H

#include <QObject>

class Mixer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(double maxRpm READ maxRpm CONSTANT)
    Q_PROPERTY(double targetRpm READ targetRpm NOTIFY targetRpmChanged)
    Q_PROPERTY(double actualRpm READ actualRpm NOTIFY actualRpmChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(bool fault READ hasFault NOTIFY stateChanged)

public:
    enum class State
    {
        Stopped,
        Running,
        Fault
    };

    explicit Mixer(QObject *parent = nullptr);

    QString stateText() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setFault();
                void resetFault();
    Q_INVOKABLE void connect();
    Q_INVOKABLE void disconnectDevice();

    static constexpr double MaxRpm = 1000.0;

    double maxRpm() const { return MaxRpm; }
    double targetRpm() const;
    double actualRpm() const;

    void setTargetRpm(double rpm);
    void simulateStep(double elapsedSeconds);

    bool isConnected() const;
    bool isRunning() const;
    bool hasFault() const;

    State state() const;

private:

    State m_state;
    bool m_connected;
    double m_targetRpm;
    double m_actualRpm;

    void clearRpm();

signals:
    void stateChanged();
    void connectionChanged();
    void targetRpmChanged();
    void actualRpmChanged();
};

#endif // MIXER_H
