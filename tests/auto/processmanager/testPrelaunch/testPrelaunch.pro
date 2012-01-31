CONFIG -= app_bundle
include(../processmanager.pri)
include(../../../../src/core/core.pri)

LIBS += -L../../../../src/core

DESTDIR = ./
SOURCES = main.cpp
TARGET  = testPrelaunch

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testPrelaunch
INSTALLS += target
