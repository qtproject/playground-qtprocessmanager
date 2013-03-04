CONFIG -= app_bundle
QT += processmanager jsonstream
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES += testPipeLauncher.cpp
TARGET  = testPipeLauncher

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testPipeLauncher
INSTALLS += target
