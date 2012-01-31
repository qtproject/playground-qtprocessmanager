TEMPLATE = lib
TARGET = processmanager-launcher

QT += jsonstream
LIBS += -L../core

include($$PWD/../../config.pri)
include(../core/core.pri)
include(launcher-lib.pri)

mac:!staticlib {
    QMAKE_POST_LINK = install_name_tool -id $$PWD/${TARGET} ${TARGET}
}

headers.path = $$INSTALLBASE/include/qtprocessmanager
headers.files = $$HEADERS

target.path = $$INSTALLBASE/lib

INSTALLS += target headers
