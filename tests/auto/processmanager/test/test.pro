CONFIG += testcase
macx:CONFIG -= app_bundle

QT += core network declarative testlib processmanager
QT -= gui

include(../processmanager.pri)

SOURCES = ../tst_processmanager.cpp
TARGET = ../$$TESTCASE_NAME

OTHER_FILES +=

testDataFiles.files = data
testDataFiles.path = .
DEPLOYMENT += testDatafiles
