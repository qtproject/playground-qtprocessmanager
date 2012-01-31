TEMPLATE = lib
TARGET = processmanager-declarative

LIBS += -L../core

include($$PWD/../../config.pri)
include(../core/core.pri)
include(declarative-lib.pri)

SOURCES += \
  plugin.cpp

mac:!staticlib {
    QMAKE_POST_LINK = install_name_tool -id $$PWD/${TARGET} ${TARGET}
}

qmldir.path = $$INSTALLBASE/imports/com/nokia/QtProcessManager
qmldir.files += $$PWD/qmldir

headers.path = $$INSTALLBASE/include/qtprocessmanager
headers.files = $$HEADERS

target.path = $$INSTALLBASE/lib

INSTALLS += target headers qmldir
