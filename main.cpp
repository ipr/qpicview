#include <QtGui/QApplication>
#include "mainwindow.h"

//#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

	// TODO: if file name is given, hide "file open" toolbar?
	// also, add switch to fullscreen,
	// get folder location
	// and make "playlist" for switching files,
	// finally, zoom in/out and scrolling

	// TODO: menubar/titlebar styling?
	//QStringList cmdline_args = a.arguments();
	
    return a.exec();
}
