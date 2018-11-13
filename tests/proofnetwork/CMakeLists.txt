cmake_minimum_required(VERSION 3.10.3)
project(ProofNetworkTest LANGUAGES CXX)

proof_add_target_sources(network_test
    abstractrestserver_test.cpp
    urlquerybuilder_test.cpp
    httpdownload_test.cpp
    restclient_test.cpp
)
proof_add_target_resources(network_test tests_resources.qrc)

proof_add_test(network_test
    PROOF_LIBS Network
)
