#-------------------------------------------------
#
# Project created by QtCreator 2013-09-12T21:42:08
#
#-------------------------------------------------

QT       += network webkitwidgets xml

QT       -= gui

TARGET = ingonline
TEMPLATE = lib

CONFIG += c++11

DEFINES += INGONLINE_LIBRARY

SOURCES += AccountInfoProvider.cpp

HEADERS += AccountInfoProvider.hpp\
        ingonline_global.hpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
