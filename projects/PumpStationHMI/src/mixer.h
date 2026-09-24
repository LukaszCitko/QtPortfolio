#ifndef MIXER_H
#define MIXER_H

#include <QObject>

class Mixer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)

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
    Q_INVOKABLE void resetFault();
    Q_INVOKABLE void connect();

    bool isConnected() const;
    bool isRunning() const;

    State state() const;

private:

    State m_state;
    bool m_connected;

signals:
    void stateChanged();
    void connectionChanged();
};

#endif // MIXER_H
