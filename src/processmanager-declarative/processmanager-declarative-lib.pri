QT += qml processmanager jsonstream

CONFIG += network

INCLUDEPATH += $$PWD

HEADERS += \
  $$PWD/declarativeprocessmanager.h \
  $$PWD/declarativesocketlauncher.h \
  $$PWD/declarativematchdelegate.h \
  $$PWD/declarativerewritedelegate.h

SOURCES += \
  $$PWD/declarativeprocessmanager.cpp \
  $$PWD/declarativesocketlauncher.cpp \
  $$PWD/declarativematchdelegate.cpp \
  $$PWD/declarativerewritedelegate.cpp
