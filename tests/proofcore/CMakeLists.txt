cmake_minimum_required(VERSION 3.12.0)
project(ProofCoreTest LANGUAGES CXX)

proof_add_target_sources(core_tests
    humanizer_test.cpp
    objectscache_test.cpp
    settings_test.cpp
    proofobject_test.cpp
    proofobject_dirty_test.cpp
    memorystoragenotificationhandler_test.cpp
)
proof_add_target_resources(core_tests tests_resources.qrc)

proof_add_test(core_tests
    PROOF_LIBS Core
)
