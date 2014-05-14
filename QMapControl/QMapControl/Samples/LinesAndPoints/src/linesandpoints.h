#ifndef LINESANDPOINTS_H
#define LINESANDPOINTS_H

#include <QtGui>
#include <mapcontrol.h>
#include <osmmapadapter.h>
#include <maplayer.h>
#include <imagepoint.h>
#include <circlepoint.h>
#include <linestring.h>
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

        public slots:
                void geometryClicked(Geometry* geom, QPoint coord_px);
                void mouseEventCoordinate(const QMouseEvent * evt, const QPointF point);

        protected:
                void keyPressEvent(QKeyEvent* evnt);

};

#endif
