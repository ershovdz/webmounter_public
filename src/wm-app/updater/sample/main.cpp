#include "qttestapp.h"
#include <QtGui/QApplication>

#include "fvupdater.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	
	QtTestApp w;
	w.show();
	return a.exec();
}
