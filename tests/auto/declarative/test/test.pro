CONFIG += testcase
macx:CONFIG -= app_bundle

QT += core network declarative testlib processmanager processmanagerdeclarative

include(../declarative.pri)

SOURCES = ../tst_declarative.cpp
TARGET = ../$$TESTCASE_NAME

OTHER_FILES +=

testDataFiles.files = data
testDataFiles.path = .
DEPLOYMENT += testDatafiles
