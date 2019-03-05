// clazy:skip

#include "proofnetwork/user.h"
#include "proofnetwork/user_p.h"

#include "gtest/proof/test_global.h"

#include <QSignalSpy>

using namespace Proof;
using testing::Test;

class TestUserPrivate;
class TestUser : public User
{
    Q_DECLARE_PRIVATE(TestUser)
    Q_OBJECT
public:
    TestUser(const QString &userName, const QString &fullName, const QString &email);
    static QSharedPointer<TestUser> create(const QString &userName, const QString &fullName, const QString &email);
};

class TestUserPrivate : public UserPrivate
{
    Q_DECLARE_PUBLIC(TestUser)
    TestUserPrivate(const QString &userName) : UserPrivate(userName) {}
};

TestUser::TestUser(const QString &userName, const QString &fullName, const QString &email)
    : User(*new TestUserPrivate(userName))
{
    Q_D(TestUser);
    d->setFullName(fullName);
    d->setEmail(email);
    setFetched(true);
}

QSharedPointer<TestUser> TestUser::create(const QString &userName, const QString &fullName, const QString &email)
{
    QSharedPointer<TestUser> result(new TestUser(userName, fullName, email));
    initSelfWeakPtr(result);
    return result;
}

class UserTest : public Test
{
public:
    UserTest() {}

protected:
    void SetUp() override
    {
        userUT = TestUser::create("vadim.petrunin", "Vadim Petrunin", "vadim.petrunin@farheap.com");
        userUT2 = TestUser::create("aliya", "Aliya", "aliyai@farheap.com");

        qmlWrapperUT = userUT->toQmlWrapper();
    }

    void TearDown() override { delete qmlWrapperUT; }

protected:
    QSharedPointer<TestUser> userUT;
    QSharedPointer<TestUser> userUT2;
    UserQmlWrapper *qmlWrapperUT;
};

TEST_F(UserTest, qmlWrapperProperties)
{
    QStringList invalidProperties = findWrongChangedSignalsInQmlWrapper(qmlWrapperUT);
    EXPECT_EQ(0, invalidProperties.count()) << invalidProperties.join("\n").toLatin1().constData();
}

TEST_F(UserTest, fromJson)
{
    EXPECT_TRUE(userUT->isFetched());

    EXPECT_EQ("vadim.petrunin", userUT->userName());
    EXPECT_EQ("vadim.petrunin", qmlWrapperUT->userName());
    EXPECT_EQ("Vadim Petrunin", userUT->fullName());
    EXPECT_EQ("Vadim Petrunin", qmlWrapperUT->fullName());
    EXPECT_EQ("vadim.petrunin@farheap.com", userUT->email());
    EXPECT_EQ("vadim.petrunin@farheap.com", qmlWrapperUT->email());
}

TEST_F(UserTest, updateFrom)
{
    QVector<QSignalSpy *> spies = spiesForObject(userUT.data());
    QVector<QSignalSpy *> qmlspies = spiesForObject(qmlWrapperUT);

    userUT->updateFrom(userUT2);

    for (QSignalSpy *spy : spies)
        EXPECT_EQ(1, spy->count()) << spy->signal().constData();

    for (QSignalSpy *spy : qmlspies)
        EXPECT_EQ(1, spy->count()) << spy->signal().constData();

    qDeleteAll(spies);
    spies.clear();
    qDeleteAll(qmlspies);
    qmlspies.clear();

    EXPECT_EQ(userUT2->userName(), userUT->userName());
    EXPECT_EQ(userUT2->fullName(), userUT->fullName());
    EXPECT_EQ(userUT2->email(), userUT->email());
}

#include "user_test.moc"
