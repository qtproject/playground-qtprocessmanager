CONFIG += testcase
macx:CONFIG -= app_bundle

QT += core network declarative testlib

LIBS += -L../../../../src/core -L../../../../src/declarative

include(../declarative.pri)
include(../../../../src/declarative/declarative.pri)

SOURCES = ../tst_declarative.cpp
TARGET = ../$$TESTCASE_NAME

OTHER_FILES +=

testDataFiles.files = data
testDataFiles.path = .
DEPLOYMENT += testDatafiles
