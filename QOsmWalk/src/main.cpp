#include <QApplication>
#include "qosmwalk.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QOsmWalk ta;
    ta.resize(480,640);
    ta.setWindowTitle("QOsmWalk");
    ta.show();
    return app.exec();
}
