#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QFont>
#include <windows.h>
#include<shobjidl_core.h>

#define AppName "XGINS Test"
#define Version "v0.4.8"

static void initStyleSheet(QApplication& a,QString file)
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
	WCHAR AppUserModelID[256] = { 0 };
	DWORD currentProcessId = GetCurrentProcessId();
	//设置应用程序ID,每次启动时不同,防止系统任务栏图标合并
    swprintf_s(AppUserModelID, L"%s-%d", L"XGNSSTest", currentProcessId);
    SetCurrentProcessExplicitAppUserModelID(AppUserModelID);

    QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(true);
	initStyleSheet(a, ":/XGNSSTest/res/style.css");

	MainWindow* w = new MainWindow(nullptr, AppName, Version);
    w->show();

    return a.exec();
}
