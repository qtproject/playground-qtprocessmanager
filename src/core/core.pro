TEMPLATE = lib
TARGET = processmanager-core

include($$PWD/../../config.pri)
include($$PWD/core-lib.pri)

mac {
    QMAKE_POST_LINK = install_name_tool -id $$PWD/${TARGET} ${TARGET}
}

target.path = $$INSTALLBASE/lib

headers.path = $$INSTALLBASE/include/qtprocessmanager
headers.files = $$PUBLIC_HEADERS

INSTALLS += target headers
