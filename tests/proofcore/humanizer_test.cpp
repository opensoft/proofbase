#include "gtest/test_global.h"

#include "proofcore/helpers/humanizer.h"
#include <QtDebug>

class HumanizerTest: public ::testing::Test
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
    //FIXME: Fix humanizer and uncomment this test
    EXPECT_EQ("1w", HumanizerUT.humanizeTime(60 * 60 * 24 * 10, Proof::Humanizer::StopAtWeeks));
}

TEST_F(HumanizerTest, humanizeBytesSizeTest)
{
    //TODO: Write test
}
