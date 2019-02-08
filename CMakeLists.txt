cmake_minimum_required(VERSION 3.12.0)
if(NOT PROOF_FULL_BUILD)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/modules")
    include(ProofInternal)
endif()

project(ProofBase VERSION ${PROOF_VERSION} LANGUAGES CXX)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qca-qt5 CONFIG REQUIRED)
if(NOT PROOF_FULL_BUILD)
    proof_init()
    find_package(ProofSeed CONFIG REQUIRED)
    find_package(proof-gtest CONFIG REQUIRED)
    enable_testing()
endif()

if (NOT PROOF_CLANG_TIDY)
    add_subdirectory(3rdparty/qamqp)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProofCore)
include(ProofNetwork)

add_subdirectory(tests/proofcore)
add_subdirectory(tests/proofnetwork)
