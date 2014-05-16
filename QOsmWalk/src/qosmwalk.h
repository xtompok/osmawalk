#ifndef QOSMWALK_H
#define QOSMWALK_H

#include <QtGui>
#include <mapcontrol.h>
#include <osmmapadapter.h>
#include <maplayer.h>
#include <imagepoint.h>
#include <circlepoint.h>
#include <linestring.h>
extern "C" {
    #include "/aux/jethro/bakalarka/compiled/searchlib.h"
}
using namespace qmapcontrol;

class QOsmWalk : public QWidget
{
        Q_OBJECT
        public:
                QOsmWalk(QWidget *parent = 0);
                ~QOsmWalk();

        private:
                MapControl* mc;
                Layer * layer;
                QPoint lastPoint;
                QLabel * label;
                QPointF * firstPoint;
                QPointF * secondPoint;
                void addZoomButtons();
                void resizeEvent(QResizeEvent * evt);
                void searchPath(QPointF * first, QPointF * second);
                struct search_data_t searchData;

        public slots:
                void mouseEventCoordinate(const QMouseEvent * evt, const QPointF point);
};

#endif // QOSMWALK_H

