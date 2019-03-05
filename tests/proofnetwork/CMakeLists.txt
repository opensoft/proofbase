cmake_minimum_required(VERSION 3.12.0)
project(ProofNetworkTest LANGUAGES CXX)

proof_add_target_sources(network_tests
    abstractrestserver_test.cpp
    abstractrestserver_system_endpoints_test.cpp
    abstractrestserver_methods_test.cpp
    urlquerybuilder_test.cpp
    httpdownload_test.cpp
    restclient_test.cpp
    errormessagesregistry_test.cpp
    user_test.cpp
)
proof_add_target_resources(network_tests tests_resources.qrc)

proof_add_test(network_tests
    PROOF_LIBS Network
)
