TEMPLATE = lib
TARGET   = $$QT.processmanagerdeclarative.name
MODULE   = processmanagerdeclarative

load(qt_module)
load(qt_module_config)

DESTDIR = $$QT.processmanagerdeclarative.libs
VERSION = $$QT.processmanagerdeclarative.VERSION
DEFINES += QT_ADDON_PROCESSMANAGER_LIB

CONFIG += module create_prl
MODULE_PRI = ../../modules/qt_processmanagerdeclarative.pri

include(declarative-lib.pri)
