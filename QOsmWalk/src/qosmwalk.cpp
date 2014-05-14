#include "qosmwalk.h"
/*!
 * \example linesandpoints.cpp
 * This application demonstrates how to use Geometry objects and how to add them to a layer.
 *
 * Here are used three different point types:
 *  - One which displays a image
 *  - One which draws a plain circle
 *  - One which uses a QPen to draw a circle
 *  - One which has no markers
 * Then these Points were added to a LineString
 *
 * Also there is a keylistener.
 *
 * You can find this example here: MapAPI/Samples/LinesAndPoints
 * \image html sample_linesandpoints.png "screenshot"
 */
QOsmWalk::QOsmWalk(QWidget *parent)
    : QWidget(parent)
{
    // the size which the QMapControl should fill
    QSize size = QSize(480,640);

    mc = new MapControl(size);
    mc->enableMouseWheelEvents();

    label = new QLabel("Click for first point");
    // create layout
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(mc);
    layout->addWidget(label);
    layout->setStretch(0,1);
    layout->setStretch(1,0);
    setLayout(layout);

    // create layer
    MapAdapter* mapadapter = new OSMMapAdapter();
    layer = new MapLayer("Custom Layer", mapadapter);

    mc->addLayer(layer);

    //Initialize points
    firstPoint = NULL;
    secondPoint = NULL;

    // Connect click events of the layer to this object
    connect(mc,SIGNAL(mouseEventCoordinate(const QMouseEvent*,const QPointF)),
            this, SLOT(mouseEventCoordinate(const QMouseEvent *,const QPointF)));

    // Sets the view to the interesting area
    QList<QPointF> view;
    view.append(QPointF(8.24764, 50.0319));
    view.append(QPointF(8.28412, 49.9998));
    mc->setView(view);

    mc->setView(QPointF(14.4511 , 50.0629  ));
    mc->setZoom(13);
    mc->setMaximumSize(1000,1000);

    searchData = prepareData("/aux/jethro/bakalarka/config/speeds.yaml","/aux/jethro/bakalarka/data/praha-graph.pbf");

    addZoomButtons();
}
void QOsmWalk::resizeEvent(QResizeEvent * evt){
    mc->resize(evt->size());
}

void QOsmWalk::addZoomButtons()
{
    // create buttons as controls for zoom
    QPushButton* zoomin = new QPushButton("+");
    QPushButton* zoomout = new QPushButton("-");
    zoomin->setMaximumWidth(50);
    zoomout->setMaximumWidth(50);

    connect(zoomin, SIGNAL(clicked(bool)),
              mc, SLOT(zoomIn()));
    connect(zoomout, SIGNAL(clicked(bool)),
              mc, SLOT(zoomOut()));

    // add zoom buttons to the layout of the MapControl
    QVBoxLayout* innerlayout = new QVBoxLayout;
    innerlayout->addWidget(zoomin);
    innerlayout->addWidget(zoomout);
    mc->setLayout(innerlayout);
}

void QOsmWalk::mouseEventCoordinate(const QMouseEvent * evt, const QPointF point){
    if (evt->type()==QEvent::MouseButtonPress)
        lastPoint = evt->pos();
    if ((evt->type()==QEvent::MouseButtonRelease)&&(evt->pos()==lastPoint)){
        if (firstPoint==NULL){
            qDebug()<<"Setting first point:" << point.y() << point.x();
            firstPoint = new QPointF(point);
            label->setText("Click to select second point");

        } else if (secondPoint==NULL){
            qDebug()<<"Setting second point:"<< point.y() << point.x();
            secondPoint = new QPointF(point);
            label->setText("Searching...");
            searchPath(firstPoint,secondPoint);
        } else{
            qDebug()<<"Setting first point again:"<< point.y() << point.x();
            firstPoint = new QPointF(point);
            delete secondPoint;
            secondPoint = NULL;
            label->setText("Click to select second point");
            while (layer->getGeometries().length() > 0)
                layer->removeGeometry(layer->getGeometries()[0]);
        }

        layer->addGeometry( new CirclePoint(point.x(),point.y(),15));

        mc->updateRequestNew();
    }

}

void QOsmWalk::searchPath(QPointF * first, QPointF * second){
    struct search_result_t result;
    result = findPath(searchData,first->y(),first->x(),second->y(),second->x());

    // create a LineString
    QList<Point*> qpoints;


    for (int i=0;i<result.n_points;i++){
        qpoints.append(new Point(result.points[i].lon,result.points[i].lat));
    }

    QPen* linepen = new QPen(QColor(0, 0, 255, 100));
    linepen->setWidth(5);
    // Add the Points and the QPen to a LineString
    LineString* ls = new LineString(qpoints, "Peskobus", linepen);

    label->setText(QString("Cas: %1 min").arg(result.time/60,0,'f',1));

    free(result.points);

    // Add the LineString to the layer
    layer->addGeometry(ls);

}

QOsmWalk::~QOsmWalk()
{
}
