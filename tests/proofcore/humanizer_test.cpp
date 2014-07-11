#include "gtest/test_global.h"

#include "proofcore/helpers/humanizer.h"

using testing::TestWithParam;
using std::tuple;

class HumanizerTest: public TestWithParam<tuple<QString, qlonglong>>
{
public:
    HumanizerTest()
    {
    }

protected:
    virtual void SetUp()
    {
    }

protected:
    Proof::Humanizer HumanizerUT;
};

//Simple test example
TEST_F(HumanizerTest, humanizeTimeTest)
{
    EXPECT_EQ("0s", HumanizerUT.humanizeTime(-1));

    EXPECT_EQ("0s", HumanizerUT.humanizeTime(0));
    EXPECT_EQ("0s", HumanizerUT.humanizeTime(0, Proof::Humanizer::StopAtSeconds));
    EXPECT_EQ("<1m", HumanizerUT.humanizeTime(0, Proof::Humanizer::StopAtMinutes));
    EXPECT_EQ("<1h", HumanizerUT.humanizeTime(0, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("<1d", HumanizerUT.humanizeTime(0, Proof::Humanizer::StopAtDays));
    EXPECT_EQ("<1w", HumanizerUT.humanizeTime(0, Proof::Humanizer::StopAtWeeks));

    EXPECT_EQ("1s", HumanizerUT.humanizeTime(1));
    EXPECT_EQ("<1m", HumanizerUT.humanizeTime(1, Proof::Humanizer::StopAtMinutes));
    EXPECT_EQ("<1h", HumanizerUT.humanizeTime(1, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("<1d", HumanizerUT.humanizeTime(1, Proof::Humanizer::StopAtDays));
    EXPECT_EQ("<1w", HumanizerUT.humanizeTime(1, Proof::Humanizer::StopAtWeeks));

    EXPECT_EQ("1m", HumanizerUT.humanizeTime(60));
    EXPECT_EQ("1m 1s", HumanizerUT.humanizeTime(61));
    EXPECT_EQ("1m", HumanizerUT.humanizeTime(61, Proof::Humanizer::StopAtMinutes));
    EXPECT_EQ("1m", HumanizerUT.humanizeTime(60, Proof::Humanizer::StopAtMinutes));
    EXPECT_EQ("<1h", HumanizerUT.humanizeTime(60, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("<1d", HumanizerUT.humanizeTime(60, Proof::Humanizer::StopAtDays));
    EXPECT_EQ("<1w", HumanizerUT.humanizeTime(60, Proof::Humanizer::StopAtWeeks));

    EXPECT_EQ("1h", HumanizerUT.humanizeTime(60 * 60));
    EXPECT_EQ("1h 1m", HumanizerUT.humanizeTime(60 * 60 + 61));
    EXPECT_EQ("1h", HumanizerUT.humanizeTime(60 * 60, Proof::Humanizer::StopAtMinutes));
    EXPECT_EQ("1h", HumanizerUT.humanizeTime(60 * 60 + 61, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("1h", HumanizerUT.humanizeTime(60 * 60, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("<1d", HumanizerUT.humanizeTime(60 * 60, Proof::Humanizer::StopAtDays));
    EXPECT_EQ("<1w", HumanizerUT.humanizeTime(60 * 60, Proof::Humanizer::StopAtWeeks));

    EXPECT_EQ("1d", HumanizerUT.humanizeTime(60 * 60 * 24).toLatin1());
    EXPECT_EQ("1d", HumanizerUT.humanizeTime(60 * 60 * 24 + 1));
    EXPECT_EQ("1d 1h", HumanizerUT.humanizeTime(60 * 60 * 25 + 61));
    EXPECT_EQ("1d", HumanizerUT.humanizeTime(60 * 60 * 24, Proof::Humanizer::StopAtMinutes));
    EXPECT_EQ("1d", HumanizerUT.humanizeTime(60 * 60 * 24, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("1d 1h", HumanizerUT.humanizeTime(60 * 60 * 25, Proof::Humanizer::StopAtHours));
    EXPECT_EQ("1d", HumanizerUT.humanizeTime(60 * 60 * 24, Proof::Humanizer::StopAtDays));
    EXPECT_EQ("<1w", HumanizerUT.humanizeTime(60 * 60 * 24, Proof::Humanizer::StopAtWeeks));
    EXPECT_EQ("2w 1d", HumanizerUT.humanizeTime(60 * 60 * 24 * 15));
    EXPECT_EQ("1w", HumanizerUT.humanizeTime(60 * 60 * 24 * 10, Proof::Humanizer::StopAtWeeks));
}

//Parametrized test example
TEST_P(HumanizerTest, humanizeBytesSizeTest)
{
    QString expected = std::get<0>(GetParam());
    qlonglong value = std::get<1>(GetParam());
    EXPECT_EQ(expected, HumanizerUT.humanizeBytesSize(value));
}

INSTANTIATE_TEST_CASE_P(HumanizeBytesSizeTestParameters,
                        HumanizerTest,
                        testing::Values(tuple<QString, qlonglong>("0 bytes", 0ll),
                                        tuple<QString, qlonglong>("1 bytes", 1ll),
                                        tuple<QString, qlonglong>("512 bytes", 512ll),
                                        tuple<QString, qlonglong>("1000 bytes", 1000ll),
                                        tuple<QString, qlonglong>("1.00K", 1024ll),
                                        tuple<QString, qlonglong>("1.00K", 1025ll),
                                        tuple<QString, qlonglong>("2.00K", 2048ll),
                                        tuple<QString, qlonglong>("976.56K", 1000000ll),
                                        tuple<QString, qlonglong>("1.00M", 1048576ll),
                                        tuple<QString, qlonglong>("1.00M", 1048577ll),
                                        tuple<QString, qlonglong>("2.00M", 2097152ll),
                                        tuple<QString, qlonglong>("953.67M", 1000000000ll),
                                        tuple<QString, qlonglong>("1.00G", 1073741824ll),
                                        tuple<QString, qlonglong>("1.00G", 1073741825ll),
                                        tuple<QString, qlonglong>("2.00G", 2147483648ll)));
