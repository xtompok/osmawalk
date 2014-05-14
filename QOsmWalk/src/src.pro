#-------------------------------------------------
#
# Project created by QtCreator 2014-05-14T20:50:35
#
#-------------------------------------------------

QT       += core

#TARGET = QOsmWalk
#CONFIG   += console
#CONFIG   -= app_bundle

INCLUDEPATH += ../../QMapControl/src
unix:LIBS += -L../../QMapControl/bin -lqmapcontrol

BASE_DIR = /aux/jethro/bakalarka

QT+=network
DEPENDPATH += src ../QMapControl/src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = QOsmWalk
INCLUDEPATH += $${BASE_DIR}/compiled/include /aux/jethro/libucw/
unix:LIBS += -L$${BASE_DIR}/compiled
QMAKE_CFLAGS_DEBUG += -std=c99
QMAKE_CFLAGS_RELEASE += -std=c99
QMAKE_LFLAGS += -lyaml -lproj -lprotobuf-c

# Input
HEADERS += qosmwalk.h
SOURCES += qosmwalk.cpp main.cpp


C_SOURCE_PATH = $${BASE_DIR}/compiled/
SOURCES += $${C_SOURCE_PATH}/searchlib.c $${C_SOURCE_PATH}/writegpx.c $${C_SOURCE_PATH}/include/types.pb-c.c $${C_SOURCE_PATH}/include/graph.pb-c.c
