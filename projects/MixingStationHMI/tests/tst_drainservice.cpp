#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/batchcontroller.h"
#include "../src/database/databaseschema.h"
#include "../src/drainservice.h"
#include "../src/eventmanager.h"
#include "../src/mixer.h"
#include "../src/mixingtank.h"
#include "../src/operatorsession.h"
#include "../src/pump.h"
#include "../src/valve.h"

class TestDrainService : public QObject
{
    Q_OBJECT

private slots:
    void requiresAuthorizationAndAvailableEquipment();
};

void TestDrainService::requiresAuthorizationAndAvailableEquipment()
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    QVERIFY(database.open());

    QString error;
    QVERIFY2(initializeDatabaseSchema(database, &error), qPrintable(error));

    QSqlQuery users(database);
    QVERIFY(users.exec(
        "INSERT INTO users (user_id, display_name, role) VALUES "
        "('operator-1', 'Test Operator', 'OPERATOR'), "
        "('tech-1', 'Test Technician', 'TECHNICIAN')"));

    Pump pump1(1), pump2(2), pump3(3);
    Valve valve1(1), valve2(2), valve3(3), valve4(4);
    Mixer mixer;
    MixingTank tank;
    EventManager events;
    OperatorSession session(database);

    BatchController controller(&pump1, &valve1,
                               &pump2, &valve2,
                               &pump3, &valve3,
                               &valve4, &mixer, &tank);

    DrainService service(session, controller, tank, valve4, events);

    QVERIFY(session.selectOperator("tech-1"));
    QVERIFY(!service.requestDrain());
    QCOMPARE(controller.state(), BatchController::State::Idle);

    tank.addWater(10.0);
    QVERIFY(session.selectOperator("operator-1"));
    QVERIFY(!service.requestDrain());
    QCOMPARE(controller.state(), BatchController::State::Idle);
    QVERIFY(!valve4.isOpen());

    QVERIFY(session.selectOperator("tech-1"));
    QVERIFY(service.requestDrain());
    QCOMPARE(controller.state(), BatchController::State::Draining);
    QVERIFY(valve4.isOpen());
    QCOMPARE(tank.volume(), 10.0);
}

QTEST_GUILESS_MAIN(TestDrainService)
#include "tst_drainservice.moc"
