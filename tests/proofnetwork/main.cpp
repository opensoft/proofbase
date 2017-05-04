#include "proofcore/coreapplication.h"
#include "proofcore/logs.h"
#include "gtest/test_global.h"

int main(int argc, char **argv) {
    Proof::CoreApplication app(argc, argv, QStringLiteral("Opensoft"), QStringLiteral("proof_tests"));
    Proof::Logs::setRulesFromString(QStringLiteral("proof.*=false"));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
