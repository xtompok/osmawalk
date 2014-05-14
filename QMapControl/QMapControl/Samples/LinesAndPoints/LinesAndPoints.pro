include(../../QMapControl.pri)

QT+=network
DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = Linesandpoints
INCLUDEPATH += /aux/jethro/bakalarka/compiled/include /aux/jethro/libucw/
unix:LIBS += -L/aux/jethro/bakalarka/compiled
QMAKE_CFLAGS_DEBUG += -std=c99
QMAKE_CFLAGS_RELEASE += -std=c99
QMAKE_LFLAGS += -lyaml -lproj -lprotobuf-c

# Input
HEADERS += src/linesandpoints.h
SOURCES += src/linesandpoints.cpp src/main.cpp

C_SOURCE_PATH = /aux/jethro/bakalarka/compiled/
SOURCES += $${C_SOURCE_PATH}/searchlib.c $${C_SOURCE_PATH}/writegpx.c $${C_SOURCE_PATH}/include/types.pb-c.c $${C_SOURCE_PATH}/include/graph.pb-c.c


