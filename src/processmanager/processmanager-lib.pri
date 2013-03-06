QT += network jsonstream
QT -= gui

INCLUDEPATH += $$PWD

PUBLIC_HEADERS += \
  $$PWD/qpmprocess.h \
  $$PWD/qprocesslist.h \
  $$PWD/qprocessfrontend.h \
  $$PWD/qprocessbackend.h \
  $$PWD/qprocessbackendfactory.h \
  $$PWD/qprocessbackendmanager.h \
  $$PWD/qmatchdelegate.h \
  $$PWD/qrewritedelegate.h \
  $$PWD/qgdbrewritedelegate.h \
  $$PWD/qidledelegate.h \
  $$PWD/qtimeoutidledelegate.h \
  $$PWD/qcpuidledelegate.h \
  $$PWD/qioidledelegate.h \
  $$PWD/qinfomatchdelegate.h \
  $$PWD/qkeymatchdelegate.h \
  $$PWD/qprocessinfo.h \
  $$PWD/qprocessmanager.h \
  $$PWD/qstandardprocessbackendfactory.h \
  $$PWD/qprelaunchprocessbackendfactory.h \
  $$PWD/qremoteprocessbackendfactory.h \
  $$PWD/qpipeprocessbackendfactory.h \
  $$PWD/qsocketprocessbackendfactory.h \
  $$PWD/qpreforkprocessbackendfactory.h \
  $$PWD/qunixprocessbackend.h \
  $$PWD/qstandardprocessbackend.h \
  $$PWD/qprelaunchprocessbackend.h \
  $$PWD/qremoteprocessbackend.h \
  $$PWD/qprocessmanager-global.h \
  $$PWD/qlauncherclient.h \
  $$PWD/qpipelauncher.h \
  $$PWD/qsocketlauncher.h \
  $$PWD/qprocutils.h \
  $$PWD/qremoteprotocol.h \
  $$PWD/qforklauncher.h \
  $$PWD/qprefork.h

HEADERS += \
  $$PUBLIC_HEADERS \
  $$PWD/qunixsandboxprocess_p.h

SOURCES += \
  $$PWD/qpmprocess.cpp \
  $$PWD/qprocessfrontend.cpp \
  $$PWD/qprocessbackend.cpp \
  $$PWD/qprocessbackendfactory.cpp \
  $$PWD/qprocessbackendmanager.cpp \
  $$PWD/qmatchdelegate.cpp \
  $$PWD/qrewritedelegate.cpp \
  $$PWD/qgdbrewritedelegate.cpp \
  $$PWD/qidledelegate.cpp \
  $$PWD/qtimeoutidledelegate.cpp \
  $$PWD/qcpuidledelegate.cpp \
  $$PWD/qioidledelegate.cpp \
  $$PWD/qinfomatchdelegate.cpp \
  $$PWD/qkeymatchdelegate.cpp \
  $$PWD/qprocessinfo.cpp \
  $$PWD/qprocessmanager.cpp \
  $$PWD/qunixprocessbackend.cpp \
  $$PWD/qstandardprocessbackendfactory.cpp \
  $$PWD/qstandardprocessbackend.cpp \
  $$PWD/qunixsandboxprocess.cpp \
  $$PWD/qprelaunchprocessbackendfactory.cpp \
  $$PWD/qprelaunchprocessbackend.cpp \
  $$PWD/qremoteprocessbackend.cpp \
  $$PWD/qremoteprocessbackendfactory.cpp \
  $$PWD/qpipeprocessbackendfactory.cpp \
  $$PWD/qsocketprocessbackendfactory.cpp \
  $$PWD/qpreforkprocessbackendfactory.cpp \
  $$PWD/qlauncherclient.cpp \
  $$PWD/qpipelauncher.cpp \
  $$PWD/qsocketlauncher.cpp \
  $$PWD/qprocutils.cpp \
  $$PWD/qforklauncher.cpp \
  $$PWD/qprefork.cpp
