QT += network
CONFIG += network

INCLUDEPATH += $$PWD

HEADERS += \
  $$PWD/launcherclient.h \
  $$PWD/pipelauncher.h \
  $$PWD/socketlauncher.h

SOURCES += \
  $$PWD/launcherclient.cpp \
  $$PWD/pipelauncher.cpp \
  $$PWD/socketlauncher.cpp
