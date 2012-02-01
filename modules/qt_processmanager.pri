QT.processmanager.VERSION = 1.0.0
QT.processmanager.MAJOR_VERSION = 1
QT.processmanager.MINOR_VERSION = 0
QT.processmanager.PATCH_VERSION = 0

QT.processmanager.name = QtAddOnProcessManager
QT.processmanager.bins = $$QT_MODULE_BIN_BASE
QT.processmanager.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtAddOnProcessManager
QT.processmanager.private_includes = $$QT_MODULE_INCLUDE_BASE/QtAddOnProcessManager/$$QT.processmanager.VERSION
QT.processmanager.sources = $$QT_MODULE_BASE/src
QT.processmanager.libs = $$QT_MODULE_LIB_BASE
QT.processmanager.plugins = $$QT_MODULE_PLUGIN_BASE
QT.processmanager.imports = $$QT_MODULE_IMPORT_BASE
QT.processmanager.depends = core network jsonstream

QT_CONFIG += processmanager
