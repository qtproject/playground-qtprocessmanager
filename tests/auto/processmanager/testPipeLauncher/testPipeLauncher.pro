CONFIG -= app_bundle
QT += processmanager
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES += main.cpp
TARGET  = testPipeLauncher

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testPipeLauncher
INSTALLS += target
