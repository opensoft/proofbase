#include "gtest/test_global.h"

#include <QCoreApplication>

GTEST_API_ int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    app.setOrganizationName("Opensoft");
    app.setApplicationName("proofcore_tests");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
