QT += qml processmanager jsonstream

CONFIG += network

INCLUDEPATH += $$PWD

HEADERS += \
  $$PWD/qdeclarativeprocessmanager.h \
  $$PWD/qdeclarativesocketlauncher.h \
  $$PWD/qdeclarativematchdelegate.h \
  $$PWD/qdeclarativerewritedelegate.h

SOURCES += \
  $$PWD/qdeclarativeprocessmanager.cpp \
  $$PWD/qdeclarativesocketlauncher.cpp \
  $$PWD/qdeclarativematchdelegate.cpp \
  $$PWD/qdeclarativerewritedelegate.cpp
