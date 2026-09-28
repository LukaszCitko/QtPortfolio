#include <QtTest>

#include "../src/mixingtank.h"
#include "../src/trendrecorder.h"

class TestTrendRecorder : public QObject
{
    Q_OBJECT

private slots:
    void recordsOnlyDuringRun();
    void usesProvidedBatchId();
};

void TestTrendRecorder::recordsOnlyDuringRun()
{
    MixingTank tank;
    TrendRecorder recorder(&tank);
    QSignalSpy recordedSpy(&recorder, &TrendRecorder::sampleRecorded);

    recorder.recordNow();
    QCOMPARE(recorder.sampleCount(), 0);

    recorder.beginRun();

    QVERIFY(recorder.isRecording());
    QCOMPARE(recorder.sampleCount(), 1);
    QVERIFY(!recorder.runIdAt(0).isEmpty());
    QVERIFY(recorder.timestampMsAt(0) > 0);
    QCOMPARE(recorder.volumeAt(0), 0.0);
    QCOMPARE(recordedSpy.count(), 1);

    tank.addWater(12.0);
    tank.setTemperature(60.0);
    recorder.recordNow();

    QCOMPARE(recorder.sampleCount(), 2);
    QCOMPARE(recorder.runIdAt(1), recorder.runIdAt(0));
    QCOMPARE(recorder.volumeAt(1), 12.0);
    QCOMPARE(recorder.temperatureAt(1), 60.0);
    QCOMPARE(recordedSpy.count(), 2);

    recorder.endRun();

    QVERIFY(!recorder.isRecording());
    QCOMPARE(recorder.sampleCount(), 3);
    QCOMPARE(recordedSpy.count(), 3);

    recorder.recordNow();
    QCOMPARE(recorder.sampleCount(), 3);
}

void TestTrendRecorder::usesProvidedBatchId()
{
    MixingTank tank;
    TrendRecorder recorder(&tank);
    const QString batchId = "batch-from-controller";

    recorder.beginRun(batchId);

    QVERIFY(recorder.isRecording());
    QCOMPARE(recorder.sampleCount(), 1);
    QCOMPARE(recorder.runIdAt(0), batchId);

    tank.addWater(10.0);
    recorder.recordNow();

    QCOMPARE(recorder.runIdAt(1), batchId);

    recorder.endRun();

    QCOMPARE(recorder.runIdAt(2), batchId);
}

QTEST_GUILESS_MAIN(TestTrendRecorder)

#include "tst_trendrecorder.moc"
