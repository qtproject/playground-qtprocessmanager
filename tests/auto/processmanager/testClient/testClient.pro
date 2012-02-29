CONFIG -= app_bundle
include(../processmanager.pri)

QT += processmanager
QT -= gui

DESTDIR = ./
SOURCES = main.cpp
TARGET  = testClient

#target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testClient
#INSTALLS += target
