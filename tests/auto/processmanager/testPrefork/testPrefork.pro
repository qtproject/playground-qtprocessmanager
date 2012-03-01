CONFIG -= app_bundle
QT += processmanager jsonstream
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES += testPrefork.cpp
TARGET  = testPrefork

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testPrefork
INSTALLS += target
