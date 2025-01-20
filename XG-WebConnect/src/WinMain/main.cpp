#include "XGWebConnect.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QFont>

#define AppName "WebConnect"
#define Version "v0.1.0"

static void initStyleSheet(QApplication& a, QString file)
{
	//Q_INIT_RESOURCE(ads); // If static linked.
	QFile f(file);
	if (f.open(QFile::ReadOnly))
	{
		const QByteArray ba = f.readAll();
		f.close();
		a.setStyleSheet(QString(ba));
	}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	initStyleSheet(a, ":/XGWebConnect/res/style.css");
    XGWebConnect w;
	w.setWindowTitle(QString("%1").arg(Version));
    w.show();
    return a.exec();
}
