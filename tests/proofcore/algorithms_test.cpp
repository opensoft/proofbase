// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/proofglobal.h"

#include <QList>
#include <QSet>
#include <set>
#include <vector>
#include <QMap>

using namespace Proof;

TEST(AlgorithmsTest, filterQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate);
    EXPECT_EQ(0, result.size());
    result = algorithms::filter(testContainer, oddPredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = algorithms::filter(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result.size());

    result = algorithms::filter(testContainer, equalPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, filterQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate).toList();
    EXPECT_EQ(0, result.size());
    result = algorithms::filter(testContainer, oddPredicate).toList();
    ASSERT_EQ(5, result.size());
    std::sort(result.begin(), result.end());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = algorithms::filter(testContainer, bigValuePredicate).toList();
    ASSERT_EQ(0, result.size());

    result = algorithms::filter(testContainer, equalPredicate).toList();
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, filterQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QMap<int, bool> result;

    auto oddPredicate = [](int, bool y)->bool{return y;};
    auto bigValuePredicate = [](int x, bool)->bool{return x > 42;};
    auto equalPredicate = [](int x, bool)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate);
    EXPECT_EQ(0, result.size());
    result = algorithms::filter(testContainer, oddPredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_TRUE(result.contains(1));
    EXPECT_TRUE(result.contains(3));
    EXPECT_TRUE(result.contains(5));
    EXPECT_TRUE(result.contains(7));
    EXPECT_TRUE(result.contains(9));

    result = algorithms::filter(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result.size());

    result = algorithms::filter(testContainer, equalPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(5));
}

TEST(AlgorithmsTest, filterVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate);
    EXPECT_EQ(0u, result.size());
    result = algorithms::filter(testContainer, oddPredicate);
    ASSERT_EQ(5u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = algorithms::filter(testContainer, bigValuePredicate);
    ASSERT_EQ(0u, result.size());

    result = algorithms::filter(testContainer, equalPredicate);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, filterSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    auto setToVector = [](const std::set<int> x)->std::vector<int> {
        std::vector<int> result;
        std::copy(x.begin(), x.end(), std::back_inserter(result));
        return result;
    };

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = setToVector(algorithms::filter(emptyContainer, oddPredicate));
    EXPECT_EQ(0u, result.size());
    result = setToVector(algorithms::filter(testContainer, oddPredicate));
    ASSERT_EQ(5u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = setToVector(algorithms::filter(testContainer, bigValuePredicate));
    ASSERT_EQ(0u, result.size());

    result = setToVector(algorithms::filter(testContainer, equalPredicate));
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, findIfQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_EQ(1, result);

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, findIfQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_TRUE((QSet<int>{1, 3, 5, 7, 9}).contains(result));

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, findIfQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QPair<int, bool> result;

    auto oddPredicate = [](int, bool y)->bool{return y;};
    auto bigValuePredicate = [](int x, bool)->bool{return x > 42;};
    auto equalPredicate = [](int x, bool)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, qMakePair(-1, false));
    EXPECT_EQ(-1, result.first);
    EXPECT_FALSE(result.second);
    result = algorithms::findIf(testContainer, oddPredicate, qMakePair(-1, false));
    ASSERT_EQ(1, result.first);
    ASSERT_TRUE(result.second);

    result = algorithms::findIf(testContainer, bigValuePredicate, qMakePair(-1, false));
    EXPECT_EQ(-1, result.first);
    EXPECT_FALSE(result.second);
    result = algorithms::findIf(testContainer, bigValuePredicate, qMakePair(50, true));
    EXPECT_EQ(50, result.first);
    EXPECT_TRUE(result.second);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    EXPECT_EQ(0, result.first);
    EXPECT_FALSE(result.second);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result.first);
    EXPECT_TRUE(result.second);
}

TEST(AlgorithmsTest, findIfVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_EQ(1, result);

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, findIfSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_EQ(1, result);

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, eraseIfQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> result;

    auto evenPredicate = [](int x)->bool{return !(x % 2);};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto smallValuePredicate = [](int x)->bool{return x < 42;};
    auto nonEqualPredicate = [](int x)->bool{return x != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, evenPredicate);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::eraseIf(result, evenPredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9, result.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, result[i]);

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, eraseIfVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    auto evenPredicate = [](int x)->bool{return !(x % 2);};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto smallValuePredicate = [](int x)->bool{return x < 42;};
    auto nonEqualPredicate = [](int x)->bool{return x != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, evenPredicate);
    EXPECT_EQ(0u, result.size());
    result = testContainer;
    algorithms::eraseIf(result, evenPredicate);
    ASSERT_EQ(5u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9u, result.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, result[i]);

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0u, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, eraseIfQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QMap<int, bool> result;

    auto falseValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(key); return !value;};
    auto bigValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key > 42;};
    auto smallValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key < 42;};
    auto nonEqualPredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_TRUE(result.contains(1));
    EXPECT_TRUE(result.contains(3));
    EXPECT_TRUE(result.contains(5));
    EXPECT_TRUE(result.contains(7));
    EXPECT_TRUE(result.contains(9));

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9, result.size());

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(5));
}

TEST(AlgorithmsTest, eraseIfQHash)
{
    QHash<int, bool> emptyContainer;
    QHash<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QHash<int, bool> result;

    auto falseValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(key); return !value;};
    auto bigValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key > 42;};
    auto smallValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key < 42;};
    auto nonEqualPredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_TRUE(result.contains(1));
    EXPECT_TRUE(result.contains(3));
    EXPECT_TRUE(result.contains(5));
    EXPECT_TRUE(result.contains(7));
    EXPECT_TRUE(result.contains(9));

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9, result.size());

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(5));
}
