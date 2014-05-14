#include "linesandpoints.h"
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
LinesAndPoints::LinesAndPoints(QWidget *parent)
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

	// create a LineString
	QList<Point*> points;
	// Points with image
	points.append(new ImagePoint(8.259959, 50.001781, "images/bus_stop.png", "Mainz, Hauptbahnhof", Point::BottomLeft));
	points.append(new ImagePoint(8.263758, 49.998917, "images/bus_stop.png", "Mainz, Münsterplatz", Point::BottomLeft));
	points.append(new ImagePoint(8.265812, 50.001952, "images/bus_stop.png","Mainz, Neubrunnenplatz", Point::BottomLeft));
	// Points with a circle
	points.append(new CirclePoint(8.2688, 50.004015, "Mainz, Bauhofstraße LRP", Point::Middle));
	points.append(new CirclePoint(8.272845, 50.00495, "Mainz, Landtag", Point::Middle));
	points.append(new CirclePoint(8.280349, 50.008173, "Mainz, Brückenkopf", Point::Middle));
	// A QPen can be used to customize the 
	QPen* pointpen = new QPen(QColor(0,255,0));
	pointpen->setWidth(3);
	points.append(new CirclePoint(8.273573, 50.016315, 15, "Wiesbaden-Mainz-Kastel, Eleonorenstraße", Point::Middle, pointpen));
	points.append(new CirclePoint(8.275145, 50.016992, 15, "Wiesbaden-Mainz-Kastel, Johannes-Goßner-Straße", Point::Middle, pointpen));
	points.append(new CirclePoint(8.270476, 50.021426, 15, "Wiesbaden-Mainz-Kastel, Ruthof", Point::Middle, pointpen));
	// "Blind" Points
	points.append(new Point(8.266445, 50.025913, "Wiesbaden-Mainz-Kastel, Mudra Kaserne"));
	points.append(new Point(8.260378, 50.030345, "Wiesbaden-Mainz-Amoneburg, Dyckerhoffstraße"));

	// A QPen also can use transparency
	QPen* linepen = new QPen(QColor(0, 0, 255, 100));
	linepen->setWidth(5);
	// Add the Points and the QPen to a LineString 
	LineString* ls = new LineString(points, "Busline 54", linepen);
	
	// Add the LineString to the layer
    layer->addGeometry(ls);
	
	// Connect click events of the layer to this object
    connect(layer, SIGNAL(geometryClicked(Geometry*, QPoint)),
			  this, SLOT(geometryClicked(Geometry*, QPoint)));

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
void LinesAndPoints::resizeEvent(QResizeEvent * evt){
    mc->resize(evt->size());
}

void LinesAndPoints::addZoomButtons()
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

void LinesAndPoints::mouseEventCoordinate(const QMouseEvent * evt, const QPointF point){
    if (evt->type()==QEvent::MouseButtonPress)
        lastPoint = evt->pos();
    if ((evt->type()==QEvent::MouseButtonRelease)&&(evt->pos()==lastPoint)){
        qDebug() << evt->type();
        qDebug() << point.x() << "," << point.y();

        if (firstPoint==NULL){
            qDebug()<<"Setting first point";
            firstPoint = new QPointF(point);
            label->setText("Click to select second point");

        } else if (secondPoint==NULL){
            qDebug()<<"Setting second point";
            secondPoint = new QPointF(point);
            label->setText("Searching...");
            searchPath(firstPoint,secondPoint);
        } else{
            qDebug()<<"Setting first point again";
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

void LinesAndPoints::searchPath(QPointF * first, QPointF * second){
    struct search_result_t result;
    result = findPath(searchData,first->y(),first->x(),second->y(),second->x());

    // create a LineString
    QList<Point*> qpoints;


    for (int i=0;i<result.n_points;i++){
      //  layer->addGeometry(new CirclePoint(points[i].lon,points[i].lat,5));
        qpoints.append(new Point(result.points[i].lon,result.points[i].lat));
      //  qDebug()<<points[i].lat<<" "<<points[i].lon;

    }
    // A QPen also can use transparency
    QPen* linepen = new QPen(QColor(0, 0, 255, 100));
    linepen->setWidth(5);
    // Add the Points and the QPen to a LineString
    LineString* ls = new LineString(qpoints, "Peskobus", linepen);

    label->setText(QString("Time: %1").arg(result.dist));

    free(result.points);

    // Add the LineString to the layer
    layer->addGeometry(ls);

}

void LinesAndPoints::geometryClicked(Geometry* geom, QPoint point)
{
    qDebug() << "Clicked!" << point.x() << ","<< point.y();
    qDebug() << geom->clickedPoints()[0];
    qDebug() << "parent: " << geom->parentGeometry();
	qDebug() << "Element clicked: " << geom->name();
	if (geom->hasClickedPoints())
	{
		QList<Geometry*> pp = geom->clickedPoints();
		qDebug() << "number of child elements: " << pp.size();
		for (int i=0; i<pp.size(); i++)
		{
			QMessageBox::information(this, geom->name(), pp.at(i)->name());
		}
	}
	else if (geom->GeometryType == "Point")
	{
		QMessageBox::information(this, geom->name(), "just a point");
	}
}

LinesAndPoints::~LinesAndPoints()
{
}

void LinesAndPoints::keyPressEvent(QKeyEvent* evnt)
{
	if (evnt->key() == 49 || evnt->key() == 17825792)  // tastatur '1'
	{
		mc->zoomIn();
	}
	else if (evnt->key() == 50)
	{
		mc->moveTo(QPointF(8.25, 60));
	}
	else if (evnt->key() == 51 || evnt->key() == 16777313)     // tastatur '3'
	{
		mc->zoomOut();
	}
	else if (evnt->key() == 54) // 6
	{
		mc->setView(QPointF(8,50));
	}
	else if (evnt->key() == 16777234) // left
	{
		mc->scrollLeft();
	}
	else if (evnt->key() == 16777236) // right
	{
		mc->scrollRight();
	}
	else if (evnt->key() == 16777235 ) // up
	{
		mc->scrollUp();
	}
	else if (evnt->key() == 16777237) // down
	{
		mc->scrollDown();
	}
	else if (evnt->key() == 48 || evnt->key() == 17825797) // 0
	{
		emit(close());
	}
	else
	{
		qDebug() << evnt->key() << endl;
	}
}
