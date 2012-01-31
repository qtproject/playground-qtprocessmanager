CONFIG += testcase
macx:CONFIG -= app_bundle

QT += core network declarative testlib
QT -= gui

LIBS += -L../../../../src/core

include(../processmanager.pri)
include(../../../../src/launcher/launcher.pri)

SOURCES = ../tst_processmanager.cpp
TARGET = ../$$TESTCASE_NAME

OTHER_FILES +=

testDataFiles.files = data
testDataFiles.path = .
DEPLOYMENT += testDatafiles
