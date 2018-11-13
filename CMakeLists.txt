cmake_minimum_required(VERSION 3.10.3)
if(NOT DEFINED PROOF_FULL_BUILD)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/modules")
    include(ProofInternal)
    proof_init()
endif()

project(ProofBase VERSION ${PROOF_VERSION} LANGUAGES CXX)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qca-qt5 CONFIG REQUIRED)
if(NOT DEFINED PROOF_FULL_BUILD)
    find_package(ProofSeed CONFIG REQUIRED)
    find_package(proof-gtest CONFIG REQUIRED)
    enable_testing()
endif()

add_subdirectory(3rdparty/qamqp)

add_compile_definitions(PROOF_VERSION=\"${PROJECT_VERSION}\")
add_compile_definitions(PROOF_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(PROOF_VERSION_YEAR=${PROJECT_VERSION_MINOR})
add_compile_definitions(PROOF_VERSION_MONTH=${PROJECT_VERSION_PATCH})
add_compile_definitions(PROOF_VERSION_DAY=${PROJECT_VERSION_TWEAK})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProofCore)
include(ProofNetwork)

add_subdirectory(tests/proofcore)
add_subdirectory(tests/proofnetwork)
