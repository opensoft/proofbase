TEMPLATE = lib
TARGET = ProofNetwork
QT += core network

PROOF_PRI_PATH = $$PWD/../proofboot
!exists($$PROOF_PRI_PATH/proof.pri):PROOF_PRI_PATH = $$(PROOF_PATH)
include($$PROOF_PRI_PATH/proof.pri)
CONFIG += proofcore qamqp

DEFINES += PROOF_NETWORK_LIB
msvc:DEFINES += QAMQP_SHARED

HEADERS += \
    include/proofnetwork/proofnetwork_global.h \
    include/proofnetwork/restclient.h \
    include/proofnetwork/networkdataentity.h \
    include/private/proofnetwork/networkdataentity_p.h \
    include/proofnetwork/user.h \
    include/proofnetwork/qmlwrappers/userqmlwrapper.h \
    include/proofnetwork/proofnetwork_types.h \
    include/proofnetwork/qmlwrappers/networkdataentityqmlwrapper.h \
    include/private/proofnetwork/qmlwrappers/networkdataentityqmlwrapper_p.h \
    include/private/proofnetwork/user_p.h \
    include/private/proofnetwork/qmlwrappers/userqmlwrapper_p.h \
    include/proofnetwork/abstractrestserver.h \
    include/proofnetwork/urlquerybuilder.h \
    include/private/proofnetwork/urlquerybuilder_p.h \
    include/private/proofnetwork/httpparser_p.h \
    include/proofnetwork/proofservicerestapi.h \
    include/private/proofnetwork/proofservicerestapi_p.h \
    include/proofnetwork/abstractamqpclient.h \
    include/private/proofnetwork/abstractamqpclient_p.h \
    include/proofnetwork/jsonamqpclient.h \
    include/private/proofnetwork/jsonamqpclient_p.h \
    include/proofnetwork/smtpclient.h \
    include/proofnetwork/emailnotificationhandler.h \
    include/proofnetwork/amqppublisher.h \
    include/proofnetwork/abstractamqpreceiver.h \
    include/private/proofnetwork/abstractamqpreceiver_p.h \
    include/proofnetwork/restapiconsumer.h \
    include/proofnetwork/apicall.h \
    include/proofnetwork/httpdownloader.h \
    include/proofnetwork/simplejsonamqpclient.h \
    include/proofnetwork/baserestapi.h \
    include/private/proofnetwork/baserestapi_p.h \
    include/proofnetwork/restapihelpers.h \
    include/proofnetwork/networkdataentityhelpers.h \
    include/proofnetwork/errormessagesregistry.h

SOURCES += \
    src/proofnetwork/restclient.cpp \
    src/proofnetwork/networkdataentity.cpp \
    src/proofnetwork/user.cpp \
    src/proofnetwork/qmlwrappers/userqmlwrapper.cpp \
    src/proofnetwork/qmlwrappers/networkdataentityqmlwrapper.cpp \
    src/proofnetwork/proofnetwork_init.cpp \
    src/proofnetwork/abstractrestserver.cpp \
    src/proofnetwork/urlquerybuilder.cpp \
    src/proofnetwork/httpparser.cpp \
    src/proofnetwork/proofservicerestapi.cpp \
    src/proofnetwork/abstractamqpclient.cpp \
    src/proofnetwork/jsonamqpclient.cpp \
    src/proofnetwork/smtpclient.cpp \
    src/proofnetwork/emailnotificationhandler.cpp \
    src/proofnetwork/amqppublisher.cpp \
    src/proofnetwork/abstractamqpreceiver.cpp \
    src/proofnetwork/restapiconsumer.cpp \
    src/proofnetwork/httpdownloader.cpp \
    src/proofnetwork/simplejsonamqpclient.cpp \
    src/proofnetwork/baserestapi.cpp \
    src/proofnetwork/apicall.cpp \
    src/proofnetwork/errormessagesregistry.cpp


include($$PROOF_PRI_PATH/proof_translation.pri)
