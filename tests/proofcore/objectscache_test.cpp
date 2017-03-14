#include "gtest/test_global.h"

#include "proofcore/objectscache.h"

using namespace Proof;

using testing::Test;

class DummyData;
using DummyDataSP = QSharedPointer<DummyData>;
using DummyDataWP = QWeakPointer<DummyData>;
class DummyData : public ProofObject
{
public:
    explicit DummyData(int _value) : ProofObject(0), value(_value) {}
    static DummyDataSP create()
    {
        return DummyDataSP::create(0);
    }
    int value;
};

class ObjectsCacheTest: public Test
{
public:
    ObjectsCacheTest()
    {
    }

protected:
    void TearDown() override
    {
        StrongObjectsCache<int, DummyData>::instance().clear();
        WeakObjectsCache<int, DummyData>::instance().clear();
        GuaranteedLifeTimeObjectsCache<int, DummyData>::instance().clear();
    }
};

TEST_F(ObjectsCacheTest, strongCache)
{
    ObjectsCache<int, DummyData> &cache = StrongObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());

    {
        DummyDataSP data = DummyDataSP::create(42);
        cache.add(42, data);
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(42));
        EXPECT_EQ(data, cache.value(42));
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(42, cache.value(42)->value);
}

TEST_F(ObjectsCacheTest, weakCache)
{
    ObjectsCache<int, DummyData> &cache = WeakObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());

    {
        DummyDataSP data = DummyDataSP::create(42);
        cache.add(42, data);
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(42));
        EXPECT_EQ(data, cache.value(42));
    }
    EXPECT_FALSE(cache.contains(42));
    DummyDataSP data = cache.value(42);
    EXPECT_TRUE(data.isNull());
    EXPECT_FALSE(cache.contains(42));
}

TEST_F(ObjectsCacheTest, guaranteedLifeTimeCache)
{
    GuaranteedLifeTimeObjectsCache<int, DummyData> &cache = GuaranteedLifeTimeObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());

    {
        DummyDataSP data = DummyDataSP::create(42);
        cache.add(42, data);
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(42));
        EXPECT_EQ(data, cache.value(42));
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(42, cache.value(42)->value);
}

TEST_F(ObjectsCacheTest, interCache)
{
    ObjectsCache<int, DummyData> &weakCache = WeakObjectsCache<int, DummyData>::instance();
    ObjectsCache<int, DummyData> &strongCache = StrongObjectsCache<int, DummyData>::instance();
    ObjectsCache<int, DummyData> &guaranteedLifeTimeCache = GuaranteedLifeTimeObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(weakCache.isEmpty());
    ASSERT_TRUE(strongCache.isEmpty());
    ASSERT_TRUE(guaranteedLifeTimeCache.isEmpty());

    DummyDataSP weakData = DummyDataSP::create(42);
    DummyDataSP strongData = DummyDataSP::create(142);
    DummyDataSP guaranteedLifeTimeData = DummyDataSP::create(22);
    DummyDataSP commonKeyStrongData = DummyDataSP::create(1042);
    DummyDataSP commonKeyWeakData = DummyDataSP::create(1042);
    DummyDataSP commonKeyGuaranteedLifeTimeData = DummyDataSP::create(1042);

    weakCache.add(42, weakData);
    strongCache.add(142, strongData);
    guaranteedLifeTimeCache.add(242, guaranteedLifeTimeData);
    weakCache.add(1042, commonKeyWeakData);
    strongCache.add(1042, commonKeyStrongData);
    guaranteedLifeTimeCache.add(1042, commonKeyGuaranteedLifeTimeData);

    EXPECT_EQ(strongData, weakCache.value(142, true));
    EXPECT_EQ(guaranteedLifeTimeData, weakCache.value(242, true));
    EXPECT_EQ(weakData, strongCache.value(42, true));
    EXPECT_EQ(guaranteedLifeTimeData, strongCache.value(242, true));
    EXPECT_EQ(strongData, guaranteedLifeTimeCache.value(142, true));
    EXPECT_EQ(weakData, guaranteedLifeTimeCache.value(42, true));

    EXPECT_TRUE(weakCache.value(142, false).isNull());
    EXPECT_TRUE(weakCache.value(242, false).isNull());
    EXPECT_TRUE(strongCache.value(42, false).isNull());
    EXPECT_TRUE(strongCache.value(242, false).isNull());
    EXPECT_TRUE(guaranteedLifeTimeCache.value(142, false).isNull());
    EXPECT_TRUE(guaranteedLifeTimeCache.value(42, false).isNull());

    EXPECT_EQ(commonKeyWeakData, weakCache.value(1042, true));
    EXPECT_EQ(commonKeyStrongData, strongCache.value(1042, true));
    EXPECT_EQ(commonKeyGuaranteedLifeTimeData, guaranteedLifeTimeCache.value(1042, true));
}
