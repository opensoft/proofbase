#include "proofcore/coreapplication.h"
#include "proofcore/logs.h"

#include "gtest/proof/test_global.h"

#include <QTimer>

int main(int argc, char **argv)
{
    Proof::CoreApplication app(argc, argv, QStringLiteral("Opensoft"), QStringLiteral("proof_tests"));
    Proof::Logs::setRulesFromString(QStringLiteral("proof.*=false"));
    QTimer::singleShot(1, &app, &Proof::CoreApplication::postInit);
    qApp->processEvents();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
