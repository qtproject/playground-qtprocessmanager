TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = src tests

include(doc/doc.pri)

system($$PWD/include/qtprocessmanager/syncheaders.sh)
