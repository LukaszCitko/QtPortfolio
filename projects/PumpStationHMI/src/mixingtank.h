#ifndef MIXINGTANK_H
#define MIXINGTANK_H

#include <QObject>

    class MixingTank : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double capacity READ capacity CONSTANT)
    Q_PROPERTY(double volume READ volume NOTIFY volumeChanged)
    Q_PROPERTY(double levelPercent READ levelPercent NOTIFY volumeChanged)
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(double waterVolume READ waterVolume NOTIFY compositionChanged)
    Q_PROPERTY(double concentrateVolume READ concentrateVolume NOTIFY compositionChanged)
    Q_PROPERTY(QString stateText READ stateText NOTIFY stateChanged)

public:
    enum class State { Empty,
                       Filling,
                       ReadyForMixing,
                       Mixing,
                       ReadyForTransfer,
                       Transferring,
                       Complete,
                       Fault
    };
    Q_ENUM(State)

    explicit MixingTank(QObject *parent = nullptr);

    double capacity() const;
    double volume() const;
    double levelPercent() const;
    double temperature() const;

    double waterVolume() const;
    double concentrateVolume() const;

    State state() const;
    QString stateText() const;

    void addWater(double amount);
    void addConcentrate(double amount);
    void removeProduct(double amount);
    bool resetAfterTransfer();

    void setTemperature(double newTemperature);
    void setState(State newState);


signals:
    void volumeChanged();
    void temperatureChanged();
    void compositionChanged();
    void stateChanged();

private:
    static constexpr double Capacity = 120.0;

    double m_volume;
    double m_temperature;
    double m_waterVolume;
    double m_concentrateVolume;
    State m_state;
};

#endif