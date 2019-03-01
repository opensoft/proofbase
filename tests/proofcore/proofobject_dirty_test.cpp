// clazy:skip

#include "proofcore/proofobject.h"
#include "proofcore/proofobject_p.h"

#include "gtest/proof/test_global.h"

using namespace Proof;

class TestDirtySecondProofObject;
class TestDirtyProofObjectPrivate;
class TestDirtyProofObject : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TestDirtyProofObject)
public:
    TestDirtyProofObject();

    int readMember() const;
    int readMemberNonConst();
    void writeMember();

    int readOtherMember() const;
    int readOtherMemberNonConst();
    void writeOtherMember();

    int readOtherListMember(int i) const;
    int readOtherListMemberNonConst(int i);
    void writeOtherListMember(int i);
};

class TestDirtyProofObjectPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(TestDirtyProofObject)
    TestDirtyProofObjectPrivate() { registerChildren(other, otherList); }
    int member = 0;
    QSharedPointer<TestDirtySecondProofObject> other = QSharedPointer<TestDirtySecondProofObject>::create();
    QVector<QSharedPointer<TestDirtySecondProofObject>> otherList{QSharedPointer<TestDirtySecondProofObject>::create(),
                                                                  QSharedPointer<TestDirtySecondProofObject>::create(),
                                                                  QSharedPointer<TestDirtySecondProofObject>::create()};
};

class TestDirtySecondProofObjectPrivate;
class TestDirtySecondProofObject : public ProofObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TestDirtySecondProofObject)
public:
    TestDirtySecondProofObject();

    int readMember() const;
    int readMemberNonConst();
    void writeMember();
};

class TestDirtyProofNonPrivateObject : public ProofObject
{
    Q_OBJECT
public:
    TestDirtyProofNonPrivateObject() : ProofObject(nullptr) { registerChildren(other, otherList); }

    void markAsDirty() const { markDirty(true); }
    int readOtherMember() const;
    int readOtherMemberNonConst();
    void writeOtherMember();

    int readOtherListMember(int i) const;
    int readOtherListMemberNonConst(int i);
    void writeOtherListMember(int i);

private:
    QSharedPointer<TestDirtySecondProofObject> other = QSharedPointer<TestDirtySecondProofObject>::create();
    QVector<QSharedPointer<TestDirtySecondProofObject>> otherList{QSharedPointer<TestDirtySecondProofObject>::create(),
                                                                  QSharedPointer<TestDirtySecondProofObject>::create(),
                                                                  QSharedPointer<TestDirtySecondProofObject>::create()};
};

class TestDirtySecondProofObjectPrivate : public ProofObjectPrivate
{
    Q_DECLARE_PUBLIC(TestDirtySecondProofObject)
    int member = 0;
};

TestDirtyProofObject::TestDirtyProofObject() : ProofObject(*new TestDirtyProofObjectPrivate, nullptr)
{}

int TestDirtyProofObject::readMember() const
{
    Q_D_CONST(TestDirtyProofObject);
    return d->member;
}

int TestDirtyProofObject::readMemberNonConst()
{
    Q_D_CONST(TestDirtyProofObject);
    return d->member;
}

void TestDirtyProofObject::writeMember()
{
    Q_D(TestDirtyProofObject);
    d->member = 1;
}

int TestDirtyProofObject::readOtherMember() const
{
    Q_D_CONST(TestDirtyProofObject);
    return d->other->readMember();
}

int TestDirtyProofObject::readOtherMemberNonConst()
{
    Q_D_CONST(TestDirtyProofObject);
    return d->other->readMemberNonConst();
}

void TestDirtyProofObject::writeOtherMember()
{
    Q_D(TestDirtyProofObject);
    d->other->writeMember();
}

int TestDirtyProofObject::readOtherListMember(int i) const
{
    Q_D_CONST(TestDirtyProofObject);
    return d->otherList[i]->readMember();
}

int TestDirtyProofObject::readOtherListMemberNonConst(int i)
{
    Q_D_CONST(TestDirtyProofObject);
    return d->otherList[i]->readMemberNonConst();
}

void TestDirtyProofObject::writeOtherListMember(int i)
{
    Q_D(TestDirtyProofObject);
    d->otherList[i]->writeMember();
}

TestDirtySecondProofObject::TestDirtySecondProofObject() : ProofObject(*new TestDirtySecondProofObjectPrivate, nullptr)
{}

int TestDirtySecondProofObject::readMember() const
{
    Q_D_CONST(TestDirtySecondProofObject);
    return d->member;
}

int TestDirtySecondProofObject::readMemberNonConst()
{
    Q_D_CONST(TestDirtySecondProofObject);
    return d->member;
}

void TestDirtySecondProofObject::writeMember()
{
    Q_D(TestDirtySecondProofObject);
    d->member = 1;
}

int TestDirtyProofNonPrivateObject::readOtherMember() const
{
    return other->readMember();
}

int TestDirtyProofNonPrivateObject::readOtherMemberNonConst()
{
    return other->readMemberNonConst();
}

void TestDirtyProofNonPrivateObject::writeOtherMember()
{
    other->writeMember();
}

int TestDirtyProofNonPrivateObject::readOtherListMember(int i) const
{
    return otherList[i]->readMember();
}

int TestDirtyProofNonPrivateObject::readOtherListMemberNonConst(int i)
{
    return otherList[i]->readMemberNonConst();
}

void TestDirtyProofNonPrivateObject::writeOtherListMember(int i)
{
    otherList[i]->writeMember();
}

TEST(ProofObjectDirtyTest, readConst)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readMember();
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readNonConst)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readMemberNonConst();
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, write)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->writeMember();
    EXPECT_TRUE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readOtherConst)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherMember();
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readOtherNonConst)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherMemberNonConst();
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, writeOther)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->writeOtherMember();
    EXPECT_TRUE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readOtherListConst)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherListMember(0);
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readOtherListNonConst)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherListMemberNonConst(0);
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, writeOtherList)
{
    std::unique_ptr<TestDirtyProofObject> obj{new TestDirtyProofObject};
    EXPECT_FALSE(obj->isDirty());
    obj->writeOtherListMember(0);
    EXPECT_TRUE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readNonPrivateOtherConst)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherMember();
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readNonPrivateOtherNonConst)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherMemberNonConst();
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, writeNonPrivateOther)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->writeOtherMember();
    EXPECT_TRUE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, markDirty)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->markAsDirty();
    EXPECT_TRUE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readNonPrivateOtherListConst)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherListMember(0);
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, readNonPrivateOtherListNonConst)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->readOtherListMemberNonConst(0);
    EXPECT_FALSE(obj->isDirty());
}

TEST(ProofObjectDirtyTest, writeNonPrivateOtherList)
{
    std::unique_ptr<TestDirtyProofNonPrivateObject> obj{new TestDirtyProofNonPrivateObject};
    EXPECT_FALSE(obj->isDirty());
    obj->writeOtherListMember(0);
    EXPECT_TRUE(obj->isDirty());
}

#include "proofobject_dirty_test.moc"
