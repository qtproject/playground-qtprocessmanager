CONFIG -= app_bundle
QT += processmanager
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES = main.cpp
TARGET  = testPrelaunch

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testPrelaunch
INSTALLS += target
