include(../core/core.pri)

CONFIG += network

INCLUDEPATH += $$PWD
LIBS += -L$$PWD -lprocessmanager-declarative

mac|unix {
    CONFIG += rpath_libdirs
    QMAKE_RPATHDIR += $$PWD
    QMAKE_LFLAGS += "-Wl,-rpath $$PWD"
}
