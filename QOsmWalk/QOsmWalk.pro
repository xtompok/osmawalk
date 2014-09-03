
TEMPLATE = subdirs
SUBDIRS += ../QMapControl/QMapControl src
CONFIG += ordered
src.depends=../QMapControl/QMapControl

unix:!macx:!symbian: LIBS += -L$$OUT_PWD/../QMapControl/QMapControl/src/ -lqmapcontrol

INCLUDEPATH += $$PWD/../QMapControl/src
DEPENDPATH += $$PWD/../QMapControl/src
