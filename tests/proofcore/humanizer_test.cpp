#include "gtest/test_global.h"

#include "proofcore/helpers/humanizer.h"

using testing::TestWithParam;
using std::tuple;

class HumanizerTimeTest: public TestWithParam<tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>>
{
public:
    HumanizerTimeTest()
    {
    }

protected:
    void SetUp() override
    {
    }

protected:
    Proof::Humanizer HumanizerUT;
};

class HumanizerBytesSizeTest: public TestWithParam<tuple<QString, qlonglong>>
{
public:
    HumanizerBytesSizeTest()
    {
    }

protected:
    void SetUp() override
    {
    }

protected:
    Proof::Humanizer HumanizerUT;
};

TEST_P(HumanizerTimeTest, humanizeTimeTest)
{
    QString expected = std::get<0>(GetParam());
    qlonglong value = std::get<1>(GetParam());
    Proof::Humanizer::TimeCategory timeCategory = std::get<2>(GetParam());
    EXPECT_EQ(expected, HumanizerUT.humanizeTime(value, timeCategory));
}

INSTANTIATE_TEST_CASE_P(HumanizeTimeTestParameters, HumanizerTimeTest,
                        testing::Values(/*tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("0s", -1),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("0s", 0),*/
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("0s", 0, Proof::Humanizer::StopAtSeconds),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1m", 0, Proof::Humanizer::StopAtMinutes),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1h", 0, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1d", 0, Proof::Humanizer::StopAtDays),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1w", 0, Proof::Humanizer::StopAtWeeks),

                                        //tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1s", 1),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1m", 1, Proof::Humanizer::StopAtMinutes),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1h", 1, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1d", 1, Proof::Humanizer::StopAtDays),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1w", 1, Proof::Humanizer::StopAtWeeks),

//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1m", 60),
//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1m 1s", 61),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1m", 61, Proof::Humanizer::StopAtMinutes),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1m", 60, Proof::Humanizer::StopAtMinutes),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1h", 60, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1d", 60, Proof::Humanizer::StopAtDays),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1w", 60, Proof::Humanizer::StopAtWeeks),

//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1h", 60 * 60),
//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1h 1m", 60 * 60 + 61),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1h", 60 * 60, Proof::Humanizer::StopAtMinutes),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1h", 60 * 60 + 61, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1h", 60 * 60, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1d", 60 * 60, Proof::Humanizer::StopAtDays),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1w", 60 * 60, Proof::Humanizer::StopAtWeeks),

//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d", 60 * 60 * 24),//.toLatin1(),
//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d", 60 * 60 * 24 + 1),
//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d 1h", 60 * 60 * 25 + 61),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d", 60 * 60 * 24, Proof::Humanizer::StopAtMinutes),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d", 60 * 60 * 24, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d 1h", 60 * 60 * 25, Proof::Humanizer::StopAtHours),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1d", 60 * 60 * 24, Proof::Humanizer::StopAtDays),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("<1w", 60 * 60 * 24, Proof::Humanizer::StopAtWeeks),
//                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("2w 1d", 60 * 60 * 24 * 15),
                                        tuple<QString, qlonglong, Proof::Humanizer::TimeCategory>("1w", 60 * 60 * 24 * 10, Proof::Humanizer::StopAtWeeks)));
//Parametrized test example
TEST_P(HumanizerBytesSizeTest, humanizeBytesSizeTest)
{
    QString expected = std::get<0>(GetParam());
    qlonglong value = std::get<1>(GetParam());
    EXPECT_EQ(expected, HumanizerUT.humanizeBytesSize(value));
}

INSTANTIATE_TEST_CASE_P(HumanizeBytesSizeTestParameters,
                        HumanizerBytesSizeTest,
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
