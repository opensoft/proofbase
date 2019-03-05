// clazy:skip
#include "proofnetwork/errormessagesregistry.h"

#include "gtest/proof/test_global.h"

#include <QTimeZone>

namespace Proof {
bool operator==(const ErrorInfo &l, const ErrorInfo &r)
{
    return l.message == r.message && l.userFriendly == r.userFriendly && l.proofErrorCode == r.proofErrorCode
           && l.proofModuleCode == r.proofModuleCode;
}
} // namespace Proof

using namespace Proof;

TEST(ErrorMesseagesRegistryTest, buildInfoWithoutArgs)
{
    ErrorMessagesRegistry registry(
        {ErrorInfo{10, 42, "Error Message", false}, ErrorInfo{10, 43, "Another Error Message", true}});
    EXPECT_EQ((ErrorInfo{10, 42, "Error Message", false}), registry.infoForCode(42));
    EXPECT_EQ((ErrorInfo{10, 43, "Another Error Message", true}), registry.infoForCode(43));
    EXPECT_EQ((ErrorInfo{0, 0, "Unknown error", true}), registry.infoForCode(44));
}

TEST(ErrorMesseagesRegistryTest, buildInfoWithArgs)
{
    ErrorMessagesRegistry registry({ErrorInfo{10, 11, "%2: Error Message %1", false}});
    EXPECT_EQ((ErrorInfo{10, 11, "Hi: Error Message 42", false}), registry.infoForCode(11, {"42", "Hi"}));
}

TEST(ErrorMesseagesRegistryTest, buildInfoWithExtraArgs)
{
    ErrorMessagesRegistry registry({ErrorInfo{10, 11, "%2: Error Message %1", false}});
    EXPECT_EQ((ErrorInfo{10, 11, "Hi: Error Message 42", false}), registry.infoForCode(11, {"42", "Hi", "Never shown"}));
}
