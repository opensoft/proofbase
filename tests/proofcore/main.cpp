#include "proofcore/coreapplication.h"
#include "gtest/test_global.h"

GTEST_API_ int main(int argc, char **argv) {
    Proof::CoreApplication app(argc, argv, "Opensoft", "proof_tests");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
