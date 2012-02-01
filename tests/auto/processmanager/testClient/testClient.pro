CONFIG -= app_bundle
include(../processmanager.pri)

QT += processmanager

DESTDIR = ./
SOURCES = main.cpp
TARGET  = testClient

#target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testClient
#INSTALLS += target
