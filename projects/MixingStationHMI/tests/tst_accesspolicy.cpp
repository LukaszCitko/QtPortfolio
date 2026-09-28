#include <QtTest>

#include "../src/accesspolicy.h"

class TestAccessPolicy : public QObject
{
    Q_OBJECT

private slots:
    void appliesRoleHierarchy();
};

void TestAccessPolicy::appliesRoleHierarchy()
{
    using Role = AccessPolicy::Role;

    QCOMPARE(AccessPolicy::roleFromText("OPERATOR"), Role::Operator);
    QCOMPARE(AccessPolicy::roleFromText("TECHNICIAN"), Role::Technician);
    QCOMPARE(AccessPolicy::roleFromText("ADMIN"), Role::Admin);
    QCOMPARE(AccessPolicy::roleFromText("UNKNOWN"), Role::None);

    QVERIFY(!AccessPolicy::canControlBatch(Role::None));
    QVERIFY(AccessPolicy::canControlBatch(Role::Operator));
    QVERIFY(AccessPolicy::canControlBatch(Role::Technician));
    QVERIFY(AccessPolicy::canControlBatch(Role::Admin));

    QVERIFY(!AccessPolicy::canResetFault(Role::Operator));
    QVERIFY(AccessPolicy::canResetFault(Role::Technician));
    QVERIFY(AccessPolicy::canResetFault(Role::Admin));

    QVERIFY(!AccessPolicy::canManageUsers(Role::Operator));
    QVERIFY(!AccessPolicy::canManageUsers(Role::Technician));
    QVERIFY(AccessPolicy::canManageUsers(Role::Admin));

    QVERIFY(!AccessPolicy::canApproveDrain(Role::None));
    QVERIFY(!AccessPolicy::canApproveDrain(Role::Operator));
    QVERIFY(AccessPolicy::canApproveDrain(Role::Technician));
    QVERIFY(AccessPolicy::canApproveDrain(Role::Admin));
}

QTEST_GUILESS_MAIN(TestAccessPolicy)
#include "tst_accesspolicy.moc"
