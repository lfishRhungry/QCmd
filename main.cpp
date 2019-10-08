#include "QCmd.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCmd w;
	w.show();
	return a.exec();
}
