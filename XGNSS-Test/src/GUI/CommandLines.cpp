#include "CommandLines.h"
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDebug>
#include "custom/QListWidgetWithMenu.h"

CommandLines::CommandLines(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.add_btn, SIGNAL(clicked()),this,SLOT(onAddClicked()));
	connect(ui.clear_btn, SIGNAL(clicked()),this,SLOT(onClearClicked()));
	connect(ui.Uboard_NMEA, SIGNAL(clicked()),this,SLOT(onUboard_NMEAClicked()));
	connect(ui.HSD_PORTU, SIGNAL(clicked()),this,SLOT(onHSD_PORTUClicked()));
	connect(ui.HSD_PORTI, SIGNAL(clicked()),this,SLOT(onHSD_PORTIClicked()));
	connect(ui.cmd_list, &QListWidget::itemDoubleClicked, this, &CommandLines::onDoubleClicked);
	connect(ui.history_list, &QListWidget::itemDoubleClicked, this, &CommandLines::onDoubleClicked);

	ui.cmd_list->setToolTip("You can double click the command to add the command into the send command box.");
	ui.history_list->setToolTip("You can double click the history to add the history into the send command box.");
	ui.add_btn->setToolTip("You can add any command you need into this commands list.");
	ui.clear_btn->setToolTip("Click this button. You can clear the histories int this list.");
}

CommandLines::~CommandLines()
{
}

void CommandLines::saveCommands()
{
	QFile file("./commands.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	QTextStream out(&file);
	for (int i = 0; i < ui.cmd_list->count(); i++) {
		if (ui.cmd_list->item(i)) {
			out << ui.cmd_list->item(i)->text() << endl;
		}
	}
}

void CommandLines::loadCommands()
{
	QFile file("./commands.txt");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		if (line.isEmpty())
			continue;
		ui.cmd_list->addItem(line);
	}
}

void CommandLines::saveHistory()
{
	QFile file("./history.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;
	QTextStream out(&file);
	for (int i = 0; i < ui.history_list->count(); i++) {
		if (ui.history_list->item(i)) {
			out << ui.history_list->item(i)->text() << endl;
		}		
	}
}

void CommandLines::loadHistory()
{
	QFile file("./history.txt");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		if (line.isEmpty())
			continue;
		ui.history_list->addItem(line);
	}
}

void CommandLines::onClearClicked()
{
	ui.history_list->clear();
}

void CommandLines::onDoubleClicked()
{
	if (ui.tabWidget->currentIndex() == 0) {
		QListWidgetItem* currentItem = ui.cmd_list->currentItem();
		if (currentItem != nullptr) {
			// 获取当前项的文本内容
			const QString itemText = currentItem->text();
			emit sgnAddCmd(itemText);
		}
	}
	else if (ui.tabWidget->currentIndex() == 1) {
		QListWidgetItem* currentItem = ui.history_list->currentItem();
		if (currentItem != nullptr) {
			// 获取当前项的文本内容
			const QString itemText = currentItem->text();
			emit sgnAddCmd(itemText);
		}
	}
}

void CommandLines::onAddHistory(QString cmd)
{
	ui.history_list->insertItem(0,cmd);
	if (ui.history_list->count() > 50) {
		ui.history_list->takeItem(50);
	}
}

void CommandLines::onAddClicked()
{
	bool isbool;
	//用于打开一个输入对话框，让用户输入文本 。
	/*parent：指定输入对话框的父窗口，可以为nullptr。
	title：指定输入对话框的标题。
	label：指定要显示在输入框上面的文本标签。
	echoMode：指定输入框上所输入的内容是否应该被隐藏。它是一个枚举值，有四个选项：QLineEdit::Normal（正常模式）、QLineEdit::NoEcho（不回显）、QLineEdit::Password（密码模式）、QLineEdit::PasswordEchoOnEdit（编辑时回显密码）。
	text：指定输入框默认显示的文本。如果你想让用户输入新的值，可以将此参数设置为空字符串（""）。
	*/
	QString cmd = QInputDialog::getText(this, "Add command", "command:", QLineEdit::Normal, "", &isbool);
	if (isbool && !cmd.isEmpty())  //判断字符串是否为空 是否点击了确认
	{
		ui.cmd_list->addItem(cmd);
	}
}

//onUboard_NMEAClicked
void CommandLines::onUboard_NMEAClicked()
{
	// $ji
	// $jshow
	// $ji,bds
	// $jsysver,sub
	// $jk,show
	// $japp
	// $jhtype,show
	// $jsignal,include
	// $jsignal,exclude
	// $jprn,include
	// $jprn,exclude
	// $jatt,htau
	// $jatt,ptau
	// $jatt,gyroaid
	// $jatt,tiltaid
	// $jatt,movebase
	// $jatt,msep
	// $jasc,gpgga,1 
	// $jasc,gpgst,1 
	// $jasc,gphpr,1 
	// $jasc,psat,rtkstat,1 
	// $jasc,psat,attstat,1
	// $jsave
	QString cmds = "$ji\n$jshow\n$ji,bds\n$jsysver,sub\n$jk,show\n$japp\n$jhtype,show\n$jsignal,include\n$jsignal,exclude\n$jprn,include\n$jprn,exclude\n$jatt,htau\n$jatt,ptau\n$jatt,gyroaid\n$jatt,tiltaid\n$jatt,movebase\n$jatt,msep\n$jasc,gpgga,1\n$jasc,gpgst,1\n$jasc,gphpr,1\n$jasc,psat,rtkstat,1\n$jasc,psat,attstat,1\n$jsave";
	emit sgnSendCmd(cmds);
}
//onHSD_PORTUClicked
void CommandLines::onHSD_PORTUClicked()
{
	// $ji,portu
	// $japp,portu
	// $jhtype,show,portu
	// $jatt,htau,portu
	// $jatt,ptau,portu
	// $jatt,gyroaid,portu
	// $jatt,tiltaid,portu
	// $jfpga,dmsg,99,portu
	// $jk,show,portu
	// $jdiff,include,portu
	// $jdiff,exclude,portu
	// $jsignal,include,portu
	// $jsignal,exclude,portu
	// $jshow,portu
	// $jmode,base,portu
	// $jatt,movebase,portu
	// $jmode,base,no,portu
	// $jasc,rtcm3,1,portu
	// $jasc,psat,rtkstat,1,portu
	// $jasc,psat,attstat,1,portu
	// $jasc,psat,cpustat,1,portu
	// $jasc,gpgga,1,portu
	// $jasc,gphpr,1,portu
	// $jasc,gpgst,1,portu
	// $jasc,d1,1,portu
	// $jbin,95,1,portu
	// $jbin,94,1,portu
	// $jbin,65,1,portu
	// $jbin,35,1,portu
	// $jbin,62,1,portu
	// $jbin,80,1,portu
	// $jbin,97,1,portu
	// $jbin,25,1,portu
	// $jbin,45,1,portu
	// $jbin,16,1,portu
	// $jbin,101,1,portu
	// $jbin,135,1,portu
	// $jbin,136,1,portu
	// $jbin,137,1,portu
	// $jephout,60,portu
	// $jsave,portu
	QString cmds = "$ji,portu\n$japp,portu\n$jhtype,show,portu\n$jatt,htau,portu\n$jatt,ptau,portu\n$jatt,gyroaid,portu\n$jatt,tiltaid,portu\n$jfpga,dmsg,99,portu\n$jk,show,portu\n$jdiff,include,portu\n$jdiff,exclude,portu\n$jsignal,include,portu\n$jsignal,exclude,portu\n$jshow,portu\n$jmode,base,portu\n$jatt,movebase,portu\n$jmode,base,no,portu\n$jasc,rtcm3,1,portu\n$jasc,psat,rtkstat,1,portu\n$jasc,psat,attstat,1,portu\n$jasc,psat,cpustat,1,portu\n$jasc,gpgga,1,portu\n$jasc,gphpr,1,portu\n$jasc,gpgst,1,portu\n$jasc,d1,1,portu\n$jbin,95,1,portu\n$jbin,94,1,portu\n$jbin,65,1,portu\n$jbin,35,1,portu\n$jbin,62,1,portu\n$jbin,80,1,portu\n$jbin,97,1,portu\n$jbin,25,1,portu\n$jbin,45,1,portu\n$jbin,16,1,portu\n$jbin,101,1,portu\n$jbin,135,1,portu\n$jbin,136,1,portu\n$jbin,137,1,portu\n$jephout,60,portu\n$jsave,portu";
	emit sgnSendCmd(cmds);
}
//onHSD_PORTIClicked
void CommandLines::onHSD_PORTIClicked()
{
	// $ji,porti
	// $japp,porti
	// $jhtype,show,porti
	// $jatt,htau,porti
	// $jatt,ptau,porti
	// $jatt,gyroaid,porti
	// $jatt,tiltaid,porti
	// $jfpga,dmsg,99,porti
	// $jk,show,porti
	// $jdiff,include,porti
	// $jdiff,exclude,porti
	// $jsignal,include,porti
	// $jsignal,exclude,porti
	// $jshow,porti
	// $jmode,base,porti
	// $jatt,movebase,porti
	// $jmode,base,no,porti
	// $jasc,rtcm3,1,porti
	// $jasc,psat,rtkstat,1,porti
	// $jasc,psat,attstat,1,porti
	// $jasc,psat,cpustat,1,porti
	// $jasc,gpgga,1,porti
	// $jasc,gphpr,1,porti
	// $jasc,gpgst,1,porti
	// $jasc,d1,1,porti
	// $jbin,95,1,porti
	// $jbin,94,1,porti
	// $jbin,65,1,porti
	// $jbin,35,1,porti
	// $jbin,62,1,porti
	// $jbin,80,1,porti
	// $jbin,97,1,porti
	// $jbin,25,1,porti
	// $jbin,45,1,porti
	// $jbin,16,1,porti
	// $jbin,101,1,porti
	// $jbin,135,1,porti
	// $jbin,136,1,porti
	// $jbin,137,1,porti
	// $jephout,60,porti
	// $jsave,porti
	QString cmds = "$ji,porti\n$japp,porti\n$jhtype,show,porti\n$jatt,htau,porti\n$jatt,ptau,porti\n$jatt,gyroaid,porti\n$jatt,tiltaid,porti\n$jfpga,dmsg,99,porti\n$jk,show,porti\n$jdiff,include,porti\n$jdiff,exclude,porti\n$jsignal,include,porti\n$jsignal,exclude,porti\n$jshow,porti\n$jmode,base,porti\n$jatt,movebase,porti\n$jmode,base,no,porti\n$jasc,rtcm3,1,porti\n$jasc,psat,rtkstat,1,porti\n$jasc,psat,attstat,1,porti\n$jasc,psat,cpustat,1,porti\n$jasc,gpgga,1,porti\n$jasc,gphpr,1,porti\n$jasc,gpgst,1,porti\n$jasc,d1,1,porti\n$jbin,95,1,porti\n$jbin,94,1,porti\n$jbin,65,1,porti\n$jbin,35,1,porti\n$jbin,62,1,porti\n$jbin,80,1,porti\n$jbin,97,1,porti\n$jbin,25,1,porti\n$jbin,45,1,porti\n$jbin,16,1,porti\n$jbin,101,1,porti\n$jbin,135,1,porti\n$jbin,136,1,porti\n$jbin,137,1,porti\n$jephout,60,porti\n$jsave,porti";
	emit sgnSendCmd(cmds);
}