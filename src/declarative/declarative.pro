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

SOURCES += \
  plugin.cpp

IMPORTNAME = QtAddOnProcessManager

qmldir.path = $$[QT_INSTALL_IMPORTS]/$$IMPORTNAME
qmldir.files += $$PWD/qmldir

INSTALLS += qmldir

# XXX will not work on Windows
system(mkdir -p $$QT_MODULE_IMPORT_BASE/$$IMPORTNAME)
system(cp qmldir $$QT_MODULE_IMPORT_BASE/$$IMPORTNAME)
