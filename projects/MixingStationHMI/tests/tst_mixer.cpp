#include <QtTest>

#include "../src/mixer.h"

class TestMixer : public QObject
{
    Q_OBJECT

private slots:
    void disconnectsOnlyWhenStopped();
    void exposesRunningAndFaultProperties();
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
void TestMixer::exposesRunningAndFaultProperties()
{
    Mixer mixer;

    QVERIFY(!mixer.isRunning());
    QVERIFY(!mixer.hasFault());
    QVERIFY(!mixer.property("running").toBool());
    QVERIFY(!mixer.property("fault").toBool());

    mixer.connect();
    mixer.start();

    QVERIFY(mixer.isRunning());
    QVERIFY(mixer.property("running").toBool());
    QVERIFY(!mixer.hasFault());

    mixer.setFault();

    QVERIFY(!mixer.isRunning());
    QVERIFY(mixer.hasFault());
    QVERIFY(mixer.property("fault").toBool());

    mixer.resetFault();

    QVERIFY(!mixer.isRunning());
    QVERIFY(!mixer.hasFault());
}

QTEST_GUILESS_MAIN(TestMixer)
#include "tst_mixer.moc"
