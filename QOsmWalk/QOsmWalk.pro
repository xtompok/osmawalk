
TEMPLATE = subdirs
SUBDIRS += ../QMapControl src
CONFIG += ordered
src.depends=../QMapControl

unix:!macx:!symbian: LIBS += -L$$OUT_PWD/../QMapControl/src/ -lqmapcontrol

INCLUDEPATH += $$PWD/../QMapControl/src
DEPENDPATH += $$PWD/../QMapControl/src
