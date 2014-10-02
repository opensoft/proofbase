#include "gtest/test_global.h"

#include "proofcore/objectscache.h"

using namespace Proof;

using testing::Test;

struct DummyData;
typedef QSharedPointer<DummyData> DummyDataSP;
typedef QWeakPointer<DummyData> DummyDataWP;
struct DummyData
{
    explicit DummyData(int _value) : value(_value) {}
    static DummyDataSP defaultObject()
    {
        static DummyDataSP result = DummyDataSP::create(0);
        return result;
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
    EXPECT_TRUE(cache.contains(42));
    DummyDataSP data = cache.value(42);
    EXPECT_TRUE(data.isNull());
    EXPECT_FALSE(cache.contains(42));
}


TEST_F(ObjectsCacheTest, interCache)
{
    ObjectsCache<int, DummyData> &weakCache = WeakObjectsCache<int, DummyData>::instance();
    ObjectsCache<int, DummyData> &strongCache = StrongObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(weakCache.isEmpty());
    ASSERT_TRUE(strongCache.isEmpty());

    DummyDataSP weakData = DummyDataSP::create(42);
    DummyDataSP strongData = DummyDataSP::create(142);
    DummyDataSP commonKeyStrongData = DummyDataSP::create(242);
    DummyDataSP commonKeyWeakData = DummyDataSP::create(242);

    weakCache.add(42, weakData);
    strongCache.add(142, strongData);
    weakCache.add(242, commonKeyWeakData);
    strongCache.add(242, commonKeyStrongData);

    EXPECT_EQ(strongData, weakCache.value(142, true));
    EXPECT_EQ(weakData, strongCache.value(42, true));
    EXPECT_TRUE(weakCache.value(142, false).isNull());
    EXPECT_TRUE(strongCache.value(42, false).isNull());
    EXPECT_EQ(commonKeyWeakData, weakCache.value(242, true));
    EXPECT_EQ(commonKeyStrongData, strongCache.value(242, true));
}
