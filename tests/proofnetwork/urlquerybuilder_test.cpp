// clazy:skip
#include "proofnetwork/urlquerybuilder.h"

#include "gtest/proof/test_global.h"

#include <QTimeZone>

using namespace Proof;

TEST(UrlQueryBuilderTest, toUrlQuery)
{
    const QDateTime now = QDateTime::currentDateTime();

    UrlQueryBuilderSP builder = UrlQueryBuilderSP::create();
    builder->setCustomParam("name1", "value");
    builder->setCustomParam("name2", qlonglong(-1));
    builder->setCustomParam("name3", qulonglong(2));
    builder->setCustomParam("name4", -3L);
    builder->setCustomParam("name5", 4UL);
    builder->setCustomParam("name6", -1);
    builder->setCustomParam("name7", 1U);
    builder->setCustomParam("name8", now);
    builder->setCustomParam("name9", true);
    builder->setCustomParam("name10", "value10");
    builder->setCustomParam("name11", 1.2);

    const QHash<QString, QString> expected{{"name1", "value"},
                                           {"name2", QString::number(qlonglong(-1))},
                                           {"name3", QString::number(qulonglong(2))},
                                           {"name4", QString::number(-3L)},
                                           {"name5", QString::number(4UL)},
                                           {"name6", QString::number(-1)},
                                           {"name7", QString::number(1U)},
                                           {"name8", now.toString("yyyy-MM-dd HH:mm:ss")
                                                         + now.timeZone()
                                                               .displayName(now, QTimeZone::OffsetName)
                                                               .replace("UTC", "")
                                                               .replace(":", "")
                                                               .replace("+", "%2B")},
                                           {"name9", "true"},
                                           {"name10", "value10"},
                                           {"name11", QString::number(1.2, 'f', 3)}};

    const QUrlQuery result = builder->toUrlQuery();

    for (const auto &queryItem : result.queryItems()) {
        QString key = queryItem.first;
        QString value = queryItem.second;
        EXPECT_EQ(expected.value(key), value) << key.toLatin1().constData();
    }
}
