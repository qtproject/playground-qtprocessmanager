CONFIG -= app_bundle
QT += processmanager jsonstream
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES += main.cpp
TARGET  = testSocketLauncher

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testSocketLauncher
INSTALLS += target
