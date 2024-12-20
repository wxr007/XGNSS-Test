#include "AboutDialog.h"
#include <QImage>

AboutDialog::AboutDialog(QWidget *parent, QString name, QString version)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle("About");
	QImage img(":/XGNSSTest/res/img/logo.png");
	//ui.logo_label->setGeometry(0, 0, 128, 128);
	QImage scalimg = img.scaled(ui.logo_label->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
	ui.logo_label->setPixmap(QPixmap::fromImage(img));
	ui.name_label->setText("Application: "+name);
	ui.version_label->setText("Version: "+version);
	setFixedSize(QSize(720, 660));
}

AboutDialog::~AboutDialog()
{}
