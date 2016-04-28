QT += webkitwidgets network printsupport

TARGET = seimiagent

DESTDIR = ../bin

SOURCES += main.cpp \
    SeimiWebPage.cpp \
    SeimiServerHandler.cpp \
    NetworkAccessManager.cpp \
    cookiejar.cpp \
    SeimiAgent.cpp \
    crashdump.cpp

HEADERS += \
    SeimiWebPage.h \
    SeimiServerHandler.h \
    NetworkAccessManager.h \
    cookiejar.h \
    SeimiAgent.h \
    crashdump.h

include(pillowcore/pillowcore.pri)
