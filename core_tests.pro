PROOF_PRI_PATH = $$PWD/../proofboot
!exists($$PROOF_PRI_PATH/proof_tests.pri):PROOF_PRI_PATH = $$(PROOF_PATH)
include($$PROOF_PRI_PATH/proof_tests.pri)

SOURCES += \
    tests/proofcore/main.cpp \
    tests/proofcore/humanizer_test.cpp \
    tests/proofcore/taskchain_test.cpp \
    tests/proofcore/objectscache_test.cpp \
    tests/proofcore/settings_test.cpp \
    tests/proofcore/proofobject_test.cpp

RESOURCES += \
    tests/proofcore/tests_resources.qrc
