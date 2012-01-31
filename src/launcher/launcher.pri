include(../core/core.pri)

CONFIG += network
QT += jsonstream

INCLUDEPATH += $$PWD
LIBS += -L$$PWD -lprocessmanager-launcher

mac|unix {
    CONFIG += rpath_libdirs
    QMAKE_RPATHDIR += $$PWD
    QMAKE_LFLAGS += "-Wl,-rpath $$PWD"
}
