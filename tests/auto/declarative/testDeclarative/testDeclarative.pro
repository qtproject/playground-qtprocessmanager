CONFIG -= app_bundle
include(../declarative.pri)

DESTDIR = ./
SOURCES = main.cpp
TARGET  = testDeclarative

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testDeclarative
INSTALLS += target
