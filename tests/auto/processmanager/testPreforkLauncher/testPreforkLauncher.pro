CONFIG -= app_bundle
QT += processmanager jsonstream
QT -= gui

include(../processmanager.pri)

DESTDIR = ./
SOURCES += testPreforkLauncher.cpp
TARGET  = testPreforkLauncher

# Position-independent code and export symbols
QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic

target.path = $$[QT_INSTALL_TESTS]/$$TESTCASE_NAME/testPreforkLauncher
INSTALLS += target
