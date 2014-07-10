#include "gtest/test_global.h"

#include <QGuiApplication>

GTEST_API_ int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
