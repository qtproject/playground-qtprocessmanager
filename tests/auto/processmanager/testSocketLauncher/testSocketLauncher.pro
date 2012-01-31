CONFIG -= app_bundle
LIBS += -L../../../../src/core

include(../processmanager.pri)
include(../../../../src/launcher/launcher.pri)

DESTDIR = ./
SOURCES += main.cpp
TARGET  = testSocketLauncher

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testSocketLauncher
INSTALLS += target
