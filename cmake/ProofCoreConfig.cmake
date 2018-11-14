include(CMakeFindDependencyMacro)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/3rdparty")
find_dependency(ZLIB REQUIRED)
find_dependency(Qt5Core CONFIG REQUIRED)
find_dependency(Qca-qt5 CONFIG REQUIRED)
find_dependency(ProofSeed CONFIG REQUIRED)
list(REMOVE_AT CMAKE_PREFIX_PATH -1)

if(NOT TARGET Proof::Core)
    include("${CMAKE_CURRENT_LIST_DIR}/ProofCoreTargets.cmake")
endif()
