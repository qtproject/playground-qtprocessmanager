CONFIG -= app_bundle
QT += processmanager
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES += testForkLauncher.cpp
TARGET  = testForkLauncher

# Position-independent code and export symbols
QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testForkLauncher
INSTALLS += target
