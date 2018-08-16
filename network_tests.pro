include (../tests/tests.pri)
QT += network
CONFIG += proofnetwork

SOURCES += \
    tests/proofnetwork/main.cpp \
    tests/proofnetwork/abstractrestserver_test.cpp \
    tests/proofnetwork/urlquerybuilder_test.cpp \
    tests/proofnetwork/httpdownload_test.cpp \
    tests/proofnetwork/restclient_test.cpp

RESOURCES += \
    tests/proofnetwork/tests_resources.qrc

DISTFILES += \
    tests/proofnetwork/data/vendor_test_body.json \
    tests/proofnetwork/data/vendor_test_body.xml
