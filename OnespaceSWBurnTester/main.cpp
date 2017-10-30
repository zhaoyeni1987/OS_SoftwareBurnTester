#include "mainframe.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	mainframe w;
	w.show();
	return a.exec();
}
