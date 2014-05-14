#ifndef LINESANDPOINTS_H
#define LINESANDPOINTS_H

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
class LinesAndPoints : public QWidget
{
        Q_OBJECT
        public:
                LinesAndPoints(QWidget *parent = 0);
                ~LinesAndPoints();

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
            //    void geometryClicked(Geometry* geom, QPoint coord_px);
                void mouseEventCoordinate(const QMouseEvent * evt, const QPointF point);

        protected:
           //     void keyPressEvent(QKeyEvent* evnt);

};

#endif
