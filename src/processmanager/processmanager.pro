TARGET   = QtAddOnProcessManager

load(qt_module)

DEFINES += QT_ADDON_PROCESSMANAGER_LIB

QMAKE_CXXFLAGS += -fPIC -fvisibility-inlines-hidden
LIBS += -ldl

linux*:!CONFIG(bionic): LIBS += -lcap

include($$PWD/processmanager-lib.pri)

mac:QMAKE_FRAMEWORK_BUNDLE_NAME = $$QT.processmanager.name
