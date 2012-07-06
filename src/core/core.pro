TEMPLATE = lib
TARGET   = $$QT.processmanager.name
MODULE   = processmanager

load(qt_module)
load(qt_module_config)

DESTDIR = $$QT.processmanager.libs
VERSION = $$QT.processmanager.VERSION
DEFINES += QT_ADDON_PROCESSMANAGER_LIB

CONFIG += module create_prl
MODULE_PRI = ../../modules/qt_processmanager.pri

QMAKE_CXXFLAGS += -fPIC -fvisibility-inlines-hidden
LIBS += -ldl

linux*:!CONFIG(bionic): LIBS += -lcap

include($$PWD/core-lib.pri)

mac:QMAKE_FRAMEWORK_BUNDLE_NAME = $$QT.processmanager.name
