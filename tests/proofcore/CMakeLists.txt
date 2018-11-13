cmake_minimum_required(VERSION 3.10.3)
project(ProofCoreTest LANGUAGES CXX)

proof_add_target_sources(core_test
    humanizer_test.cpp
    objectscache_test.cpp
    settings_test.cpp
    proofobject_test.cpp
)
proof_add_target_resources(core_test tests_resources.qrc)

proof_add_test(core_test
    PROOF_LIBS Core
)
