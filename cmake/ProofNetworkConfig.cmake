include(CMakeFindDependencyMacro)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/3rdparty")
find_dependency(Qt5Core CONFIG REQUIRED)
find_dependency(Qt5Network CONFIG REQUIRED)
find_dependency(qamqp CONFIG REQUIRED)
find_dependency(Qca-qt5 CONFIG REQUIRED)
find_dependency(ProofCore CONFIG REQUIRED)
list(REMOVE_AT CMAKE_PREFIX_PATH -1)

if(NOT TARGET Proof::Network)
    include("${CMAKE_CURRENT_LIST_DIR}/ProofNetworkTargets.cmake")
endif()
