#include <QtTest>

#include "../src/mixer.h"

class TestMixer : public QObject
{
    Q_OBJECT

private slots:
    void disconnectsOnlyWhenStopped();
};

void TestMixer::disconnectsOnlyWhenStopped()
{
    Mixer mixer;
    QSignalSpy connectionSpy(&mixer, &Mixer::connectionChanged);

    QVERIFY(!mixer.isConnected());

    mixer.connect();
    QVERIFY(mixer.isConnected());
    QCOMPARE(connectionSpy.count(), 1);

    mixer.start();
    QVERIFY(mixer.isRunning());

    mixer.disconnectDevice();
    QVERIFY(mixer.isConnected());
    QCOMPARE(connectionSpy.count(), 1);

    mixer.stop();
    mixer.disconnectDevice();
    QVERIFY(!mixer.isConnected());
    QCOMPARE(connectionSpy.count(), 2);

    mixer.start();
    QVERIFY(!mixer.isRunning());
}

QTEST_GUILESS_MAIN(TestMixer)
#include "tst_mixer.moc"
