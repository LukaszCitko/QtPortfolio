#include <QtTest>

#include "../shared/controllerprotocol.h"
#include "../src/communication/commandoutcometracker.h"

class TestCommandOutcomeTracker : public QObject
{
    Q_OBJECT

private slots:
    void acceptsOnlyMatchingResult();
    void resolvesLateAcknowledgement();
    void reportsRejectionAndDisconnect();
};

void TestCommandOutcomeTracker::acceptsOnlyMatchingResult()
{
    CommandOutcomeTracker tracker;
    const quint16 code = static_cast<quint16>(ControllerProtocol::Command::StartBatch);
    const int accepted = static_cast<int>(ControllerProtocol::CommandResult::Accepted);
    const int noResult = static_cast<int>(ControllerProtocol::CommandResult::None);

    QVERIFY(tracker.begin(code, 42));
    QVERIFY(tracker.pending());
    QVERIFY(!tracker.begin(code, 43));
    QVERIFY(!tracker.acknowledge(41, accepted).has_value());
    QVERIFY(!tracker.acknowledge(42, noResult).has_value());
    QVERIFY(tracker.pending());

    const auto result = tracker.acknowledge(42, accepted);
    QVERIFY(result.has_value());
    QVERIFY(result->outcome == CommandOutcomeTracker::Outcome::Accepted);
    QCOMPARE(result->sequence, quint16(42));
    QVERIFY(!result->late);
    QVERIFY(!tracker.pending());
    QVERIFY(tracker.begin(code, 44));
    tracker.cancelPending();
    QVERIFY(!tracker.pending());
    QVERIFY(!tracker.expire().has_value());
}

void TestCommandOutcomeTracker::resolvesLateAcknowledgement()
{
    CommandOutcomeTracker tracker;
    const quint16 code = static_cast<quint16>(ControllerProtocol::Command::StartBatch);
    const int accepted = static_cast<int>(ControllerProtocol::CommandResult::Accepted);

    QVERIFY(tracker.begin(code, 42));

    const auto expired = tracker.expire();
    QVERIFY(expired.has_value());
    QVERIFY(expired->outcome == CommandOutcomeTracker::Outcome::Unknown);
    QVERIFY(!tracker.pending());

    QVERIFY(tracker.begin(code, 43));
    QVERIFY(!tracker.pending(42));
    QVERIFY(tracker.pending(43));
    const auto late = tracker.acknowledge(42, accepted);
    QVERIFY(late.has_value());
    QVERIFY(late->outcome == CommandOutcomeTracker::Outcome::Accepted);
    QVERIFY(late->late);
    QVERIFY(tracker.pending());
}

void TestCommandOutcomeTracker::reportsRejectionAndDisconnect()
{
    CommandOutcomeTracker tracker;
    const quint16 code = static_cast<quint16>(ControllerProtocol::Command::StartBatch);
    const int invalidState = static_cast<int>(ControllerProtocol::CommandResult::InvalidState);

    QVERIFY(tracker.begin(code, 7));

    const auto rejected = tracker.acknowledge(7, invalidState);
    QVERIFY(rejected.has_value());
    QVERIFY(rejected->outcome == CommandOutcomeTracker::Outcome::Rejected);
    QCOMPARE(rejected->controllerResult, invalidState);

    QVERIFY(tracker.begin(code, 8));
    const auto disconnected = tracker.disconnect();
    QVERIFY(disconnected.has_value());
    QVERIFY(disconnected->outcome == CommandOutcomeTracker::Outcome::Unknown);
    QVERIFY(!tracker.pending());
}

QTEST_GUILESS_MAIN(TestCommandOutcomeTracker)
#include "tst_commandoutcometracker.moc"
