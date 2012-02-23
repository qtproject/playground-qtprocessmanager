CONFIG -= app_bundle
QT += processmanager

include(../processmanager.pri)

DESTDIR = ./
SOURCES += main.cpp
TARGET  = testForkLauncher

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testForkLauncher
INSTALLS += target
