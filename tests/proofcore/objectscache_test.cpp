// clazy:skip
#ifndef QT_NO_DEBUG
#    define QT_NO_DEBUG
#endif
#include "proofcore/objectscache.h"

#include "gtest/proof/test_global.h"

#include <QTest>

using namespace Proof;

using testing::Test;

class DummyData;
using DummyDataSP = QSharedPointer<DummyData>;
using DummyDataWP = QWeakPointer<DummyData>;
class DummyData : public ProofObject
{
    Q_OBJECT
public:
    explicit DummyData(int value) : ProofObject(0), value(value) {}
    static DummyDataSP create() { return DummyDataSP::create(0); }
    int value;
};

class DummyDataSimple
{
public:
    explicit DummyDataSimple(int value) : value(value) {}
    static QSharedPointer<DummyDataSimple> create() { return QSharedPointer<DummyDataSimple>::create(0); }
    int value;
};

class ObjectsCacheTest : public Test
{
public:
    ObjectsCacheTest() {}

protected:
    void TearDown() override
    {
        StrongObjectsCache<int, DummyData>::instance().clear();
        WeakObjectsCache<int, DummyData>::instance().clear();
        GuaranteedLifeTimeObjectsCache<int, DummyData>::instance().clear();
        GuaranteedLifeTimeObjectsCache<QString, DummyData>::instance().clear();
        GuaranteedLifeTimeObjectsCache<int, int>::instance().clear();
    }
};

TEST_F(ObjectsCacheTest, valueTypeNames)
{
    EXPECT_EQ("DummyData", (StrongObjectsCache<int, DummyData>::instance().valueTypeName()));
    EXPECT_EQ("", (StrongObjectsCache<int, DummyDataSimple>::instance().valueTypeName()));
    EXPECT_EQ("DummyData", (WeakObjectsCache<int, DummyData>::instance().valueTypeName()));
    EXPECT_EQ("", (WeakObjectsCache<int, DummyDataSimple>::instance().valueTypeName()));
    EXPECT_EQ("DummyData", (GuaranteedLifeTimeObjectsCache<int, DummyData>::instance().valueTypeName()));
    EXPECT_EQ("", (GuaranteedLifeTimeObjectsCache<int, DummyDataSimple>::instance().valueTypeName()));
}

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

    {
        cache.add(21, []() { return DummyDataSP::create(21); });
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(21));
        ASSERT_TRUE(cache.value(21));
        EXPECT_EQ(21, cache.value(21)->value);
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(42, cache.value(42)->value);
    EXPECT_TRUE(cache.contains(21));
    ASSERT_FALSE(cache.value(21).isNull());
    EXPECT_EQ(21, cache.value(21)->value);

    {
        auto keys = cache.keys();
        std::sort(keys.begin(), keys.end());
        EXPECT_EQ((QVector<int>{21, 42}), keys);
    }

    {
        DummyDataSP data = DummyDataSP::create(22);
        DummyDataSP actual = cache.add(21, data);
        EXPECT_NE(data, cache.value(21));
        EXPECT_EQ(21, actual->value);
    }
    DummyDataWP removed;
    {
        DummyDataSP data = DummyDataSP::create(22);
        DummyDataSP actual = cache.add(21, data, true);
        EXPECT_EQ(data, cache.value(21));
        EXPECT_EQ(22, actual->value);
        removed = data;
    }

    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_TRUE(cache.contains(21));
    ASSERT_FALSE(cache.value(21).isNull());
    cache.remove(21);
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_FALSE(cache.contains(21));
    EXPECT_FALSE(removed);

    {
        auto keys = cache.keys();
        EXPECT_EQ((QVector<int>{42}), keys);
    }
}

TEST_F(ObjectsCacheTest, weakCache)
{
    ObjectsCache<int, DummyData> &cache = WeakObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());

    DummyDataSP data42 = DummyDataSP::create(42);
    {
        cache.add(42, data42);
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(42));
        EXPECT_EQ(data42, cache.value(42));
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(42, cache.value(42)->value);

    DummyDataSP data21 = DummyDataSP::create(21);
    {
        cache.add(21, [data21]() { return data21; });
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(21));
        ASSERT_TRUE(cache.value(21));
        EXPECT_EQ(21, cache.value(21)->value);
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(42, cache.value(42)->value);
    EXPECT_TRUE(cache.contains(21));
    ASSERT_FALSE(cache.value(21).isNull());
    EXPECT_EQ(21, cache.value(21)->value);

    {
        auto keys = cache.keys();
        std::sort(keys.begin(), keys.end());
        EXPECT_EQ((QVector<int>{21, 42}), keys);
    }

    {
        DummyDataSP data = DummyDataSP::create(22);
        DummyDataSP actual = cache.add(21, data);
        EXPECT_NE(data, cache.value(21));
        EXPECT_EQ(21, actual->value);
    }
    {
        DummyDataSP data = DummyDataSP::create(22);
        DummyDataSP actual = cache.add(21, data, true);
        EXPECT_EQ(data, cache.value(21));
        EXPECT_EQ(22, actual->value);
        data21 = data;
    }

    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_TRUE(cache.contains(21));
    ASSERT_FALSE(cache.value(21).isNull());
    cache.remove(21);
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_FALSE(cache.contains(21));

    {
        auto keys = cache.keys();
        EXPECT_EQ((QVector<int>{42}), keys);
    }
}

TEST_F(ObjectsCacheTest, weakCacheWeakness)
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

    {
        cache.add(21, []() { return DummyDataSP::create(21); });
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(21));
        ASSERT_TRUE(cache.value(21));
        EXPECT_EQ(21, cache.value(21)->value);
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(42, cache.value(42)->value);
    EXPECT_TRUE(cache.contains(21));
    ASSERT_FALSE(cache.value(21).isNull());
    EXPECT_EQ(21, cache.value(21)->value);

    {
        auto keys = cache.keys();
        std::sort(keys.begin(), keys.end());
        EXPECT_EQ((QVector<int>{21, 42}), keys);
    }

    {
        DummyDataSP data = DummyDataSP::create(22);
        DummyDataSP actual = cache.add(21, data);
        EXPECT_NE(data, cache.value(21));
        EXPECT_EQ(21, actual->value);
    }
    {
        DummyDataSP data = DummyDataSP::create(22);
        DummyDataSP actual = cache.add(21, data, true);
        EXPECT_EQ(data, cache.value(21));
        EXPECT_EQ(22, actual->value);
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_TRUE(cache.contains(21));
    ASSERT_FALSE(cache.value(21).isNull());
    cache.remove(21);
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_FALSE(cache.contains(21));

    {
        auto keys = cache.keys();
        EXPECT_EQ((QVector<int>{42}), keys);
    }
}

TEST_F(ObjectsCacheTest, guaranteedLifeTimeCacheExpiration)
{
    qApp->processEvents();
    Expirator::instance()->setCleanupInterval(2);
    qApp->processEvents();
    GuaranteedLifeTimeObjectsCache<int, DummyData> &cache = GuaranteedLifeTimeObjectsCache<int, DummyData>::instance();
    cache.setObjectsMinLifeTime(1);
    GuaranteedLifeTimeObjectsCache<QString, DummyData> &longerCache =
        GuaranteedLifeTimeObjectsCache<QString, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    ASSERT_TRUE(longerCache.isEmpty());

    DummyDataWP wp;
    DummyDataWP longerWp;
    {
        DummyDataSP data = DummyDataSP::create(42);
        wp = data;
        cache.add(42, data);
        EXPECT_FALSE(cache.isEmpty());
        EXPECT_TRUE(cache.contains(42));
        EXPECT_EQ(data, cache.value(42));
    }
    EXPECT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.value(42).isNull());
    EXPECT_EQ(wp, cache.value(42));
    EXPECT_TRUE(wp);

    {
        DummyDataSP data = DummyDataSP::create(21);
        longerWp = data;
        longerCache.add("abc", data);
        EXPECT_FALSE(longerCache.isEmpty());
        EXPECT_TRUE(longerCache.contains("abc"));
        EXPECT_EQ(data, longerCache.value("abc"));
    }
    EXPECT_TRUE(longerCache.contains("abc"));
    ASSERT_FALSE(longerCache.value("abc").isNull());
    EXPECT_EQ(longerWp, longerCache.value("abc"));
    EXPECT_TRUE(longerWp);

    QTime timer;
    timer.start();
    while (cache.contains(42) && timer.elapsed() < 30000)
        qApp->processEvents();
    EXPECT_FALSE(cache.contains(42));
    EXPECT_FALSE(wp);
    EXPECT_TRUE(longerCache.contains("abc"));
    ASSERT_FALSE(longerCache.value("abc").isNull());
    EXPECT_EQ(longerWp, longerCache.value("abc"));
    EXPECT_TRUE(longerWp);
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

TEST_F(ObjectsCacheTest, strongCacheEmptyAdd)
{
    ObjectsCache<int, DummyData> &cache = StrongObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.add(1, DummyDataSP());
    EXPECT_FALSE(result);
    EXPECT_FALSE(cache.contains(1));
    ASSERT_TRUE(cache.isEmpty());
}

TEST_F(ObjectsCacheTest, weakCacheEmptyAdd)
{
    ObjectsCache<int, DummyData> &cache = WeakObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.add(1, DummyDataSP());
    EXPECT_FALSE(result);
    EXPECT_FALSE(cache.contains(1));
    ASSERT_TRUE(cache.isEmpty());
}

TEST_F(ObjectsCacheTest, guaranteedLifeTimeCacheEmptyAdd)
{
    ObjectsCache<int, DummyData> &cache = GuaranteedLifeTimeObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.add(1, DummyDataSP());
    EXPECT_FALSE(result);
    EXPECT_FALSE(cache.contains(1));
    ASSERT_TRUE(cache.isEmpty());
}

TEST_F(ObjectsCacheTest, strongCacheEmptyFetch)
{
    ObjectsCache<int, DummyData> &cache = StrongObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.value(0);
    ASSERT_TRUE(result);
    EXPECT_EQ(0, result->value);
}

TEST_F(ObjectsCacheTest, weakCacheEmptyFetch)
{
    ObjectsCache<int, DummyData> &cache = WeakObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.value(0);
    ASSERT_TRUE(result);
    EXPECT_EQ(0, result->value);
}

TEST_F(ObjectsCacheTest, guaranteedLifeTimeCacheEmptyFetch)
{
    ObjectsCache<int, DummyData> &cache = GuaranteedLifeTimeObjectsCache<int, DummyData>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.value(0);
    ASSERT_TRUE(result);
    EXPECT_EQ(0, result->value);
}

TEST_F(ObjectsCacheTest, guaranteedLifeTimeCacheDummy)
{
    ObjectsCache<int, int> &cache = GuaranteedLifeTimeObjectsCache<int, int>::instance();
    ASSERT_TRUE(cache.isEmpty());
    auto result = cache.add(5, QSharedPointer<int>::create(5));
    EXPECT_FALSE(result);
    result = cache.add(15, []() { return QSharedPointer<int>::create(15); });
    EXPECT_FALSE(result);
    EXPECT_FALSE(cache.contains(5));
    EXPECT_FALSE(cache.value(5));
    EXPECT_TRUE(cache.isEmpty());
    EXPECT_TRUE(cache.keys().isEmpty());
}
#include "objectscache_test.moc"
