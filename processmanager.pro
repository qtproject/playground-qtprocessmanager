TEMPLATE = subdirs

module_processmanager_src.subdir = src
module_processmanager_src.target = module-processmanager-src

module_processmanager_tests.subdir = tests
module_processmanager_tests.target = module-processmanager-tests
module_processmanager_tests.depends += module-processmanager-src

SUBDIRS += module_processmanager_src #module_processmanager_tests

include(doc/doc.pri)

system($$PWD/include/qtprocessmanager/syncheaders.sh)
