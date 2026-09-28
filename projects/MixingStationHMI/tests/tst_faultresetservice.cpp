#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/database/databaseschema.h"
#include "../src/eventmanager.h"
#include "../src/faultresetservice.h"
#include "../src/mixer.h"
#include "../src/operatorsession.h"
#include "../src/pump.h"

class TestFaultResetService : public QObject
{
    Q_OBJECT

private slots:
    void requiresTechnicianRole();
};

void TestFaultResetService::requiresTechnicianRole()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery insertUsers(database);
    QVERIFY(insertUsers.exec(
        "INSERT INTO users (user_id, display_name, role) VALUES "
        "('operator-1', 'Test Operator', 'OPERATOR'), "
        "('tech-1', 'Test Technician', 'TECHNICIAN')"));

    OperatorSession session(database);
    Pump pump1(1);
    Mixer mixer;
    EventManager events;
    FaultResetService service(session, pump1, mixer, events);

    pump1.setFault();
    mixer.setFault();

    QVERIFY(session.selectOperator("operator-1"));
    QVERIFY(!service.resetPump1());
    QCOMPARE(pump1.state(), Pump::State::Fault);
    QCOMPARE(events.eventLevel(0), QStringLiteral("WARNING"));

    QVERIFY(session.selectOperator("tech-1"));
    QVERIFY(service.resetPump1());
    QVERIFY(service.resetMixer());
    QCOMPARE(pump1.state(), Pump::State::Stopped);
    QCOMPARE(mixer.state(), Mixer::State::Stopped);
    QCOMPARE(events.eventCount(), 3);
    QCOMPARE(events.eventMessage(2),
    QStringLiteral("Fault reset by Test Technician"));
    pump1.setFault();
    mixer.setFault();

    QVERIFY(!service.demoResetPump1());
    QVERIFY(!service.demoResetMixer());

    QVERIFY(session.selectOperator("operator-1"));
    QVERIFY(service.demoResetPump1());
    QVERIFY(service.demoResetMixer());

    QCOMPARE(pump1.state(), Pump::State::Stopped);
    QCOMPARE(mixer.state(), Mixer::State::Stopped);
    QCOMPARE(events.eventCount(), 5);
    QCOMPARE(events.eventLevel(3), QStringLiteral("WARNING"));
    QCOMPARE(events.eventMessage(4),
    QStringLiteral("Simulation override: fault reset by Test Operator"));
}

QTEST_GUILESS_MAIN(TestFaultResetService)
#include "tst_faultresetservice.moc"
