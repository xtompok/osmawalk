include(../../QMapControl.pri)
QT+=network
DEPENDPATH += src
MOC_DIR = tmp
OBJECTS_DIR = obj
DESTDIR = ../bin
TARGET = Linesandpoints

# Input
HEADERS += src/linesandpoints.h
SOURCES += src/linesandpoints.cpp src/main.cpp

