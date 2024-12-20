#include "ControlWidget.h"
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextCursor>
#include "ThreadManager.h"
#include "LogManager.h"
#include <QFontDataBase>

#ifndef STREAM_TXT
#define STREAM_TXT    "Stream"
#endif // !STREAM_TXT

enum CtrlTabIndex {
	Options_Index =0,
	Console_Index =1
};

#define Font_Red	"#de0e12"
#define Font_Oragne "#c24c05"
#define Font_Green	"#0f7f1e"
#define Font_Blue	"#3e40dd"
#define Font_Purple "#753775"

ControlWidget::ControlWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_MsgLines = 0;
	//����ͼ������
	int fontId = QFontDatabase::addApplicationFont(":/XGNSSTest/res/fontawesome-webfont.ttf");
	QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
	QFont iconFont = QFont(fontName);
	ui.commands_btn->setFont(iconFont);
	ui.commands_btn->setText(QChar(0xf101));
	ui.pause_btn->setFont(iconFont);
	ui.pause_btn->setText(QChar(0xf04c));
	ui.clear_btn->setFont(iconFont);
	ui.clear_btn->setText(QChar(0xf014));

	m_Cmds = new CommandLines(this);
	connect(ui.clear_btn, SIGNAL(clicked()), this, SLOT(onClearClicked()));
	connect(ui.commands_btn, SIGNAL(clicked()), this, SLOT(onCmdsClicked()));
	connect(ui.send_btn, SIGNAL(clicked()), this, SLOT(onSendClicked()));
	connect(m_Cmds, SIGNAL(sgnAddCmd(const QString)), this, SLOT(onAddCmd(const QString)));
	connect(m_Cmds, SIGNAL(sgnSendCmd(const QString)), this, SLOT(onSendCmd(const QString)));
	connect(this, &ControlWidget::sgnAddHistory, m_Cmds, &CommandLines::onAddHistory);
	connect(ui.AutoSend, SIGNAL(toggled(bool)), this, SLOT(onChangeAutoSend()));
	connect(ui.pause_btn, SIGNAL(toggled(bool)), this, SLOT(onPause(bool)));
	//m_Cmds->setFixedWidth(350);
	ui.splitter_h->addWidget(m_Cmds);
	ui.splitter_h->setStretchFactor(0, 5);
	ui.splitter_h->setStretchFactor(1, 2);

	ui.splitter_v->setStretchFactor(0, 4);
	ui.splitter_v->setStretchFactor(1, 1);

	connect(this, SIGNAL(sgnWrite(const QByteArray)), ThreadManager::Instance().m_SerialThread, SIGNAL(sgnWrite(const QByteArray)), Qt::QueuedConnection);
	connect(this, SIGNAL(sgnWrite(const QByteArray)), ThreadManager::Instance().m_TcpClientThread, SIGNAL(sgnWrite(const QByteArray)), Qt::QueuedConnection);
	connect(ThreadManager::Instance().m_DataParser, SIGNAL(sgnNMEA(QByteArray)), this, SLOT(onShowDataLog(QByteArray)),Qt::QueuedConnection);
	connect(LogManager::getInstance(), SIGNAL(sgnShowMsg(int, const QString)), this, SLOT(onShowMsg(int, const QString)),Qt::QueuedConnection);
	
	ui.color_msg_txt->setFontWeight(QFont::Bold);
	ui.commands_btn->setToolTip("Hide or show the commands window.");
	ui.send_btn->setToolTip("Send the command to the stream which your choice.");
	ui.clear_btn->setToolTip("Clear all messages in the message box.");
	ui.clear_send->setToolTip("If this checkbox is checked, the contents in the command box will be cleared after each command is sent.");
	ui.pause_btn->setToolTip("Pause or resume the message box.");
	ui.pause_btn->setChecked(false);
}

ControlWidget::~ControlWidget()
{}

void ControlWidget::saveCommands()
{
	if (m_Cmds) {
		m_Cmds->saveCommands();
		m_Cmds->saveHistory();
	}
}

void ControlWidget::loadCommands()
{
	if (m_Cmds) {
		m_Cmds->loadCommands();
		m_Cmds->loadHistory();
	}
}

void ControlWidget::onCmdsClicked()
{
	if (m_Cmds->isVisible()) {
		m_Cmds->hide();
		ui.commands_btn->setText(QChar(0xf100));
	}
	else {
		m_Cmds->show();
		ui.commands_btn->setText(QChar(0xf101));
	}
}

void ControlWidget::onSendClicked()
{
	QString content = ui.send_txt->toPlainText();
	if (content.trimmed().isEmpty()) {
		return;
	}
	onSendCmd(content);
	if (ui.clear_send->isEnabled() && ui.clear_send->isChecked()) {
		ui.send_txt->clear();
	}
}

void ControlWidget::onSendCmd(const QString content)
{
	QStringList cmds = content.split("\n");
	QStringList::iterator i;
	for (i = cmds.begin(); i != cmds.end(); ++i)
	{
		QString cmd = *i;
		cmd = cmd.trimmed();
		if (cmd.isEmpty()) {
			continue;
		}
		// ����һ���µ���ɫ��ʽ
		QTextCharFormat format;
		format.setForeground(QColor(Font_Purple));  // ����������ɫ
		// ����һ���µ��ı����
		QTextCursor cursor(ui.color_msg_txt->document()->end());
		cursor.movePosition(QTextCursor::End);
		cursor.insertText("Device <: " + cmd + "\r\n", format);
		ui.color_msg_txt->moveCursor(QTextCursor::End);

		//if (ui.streambox->currentIndex() > 0) {
		//	//StreamManager::getInstance()->sendCmd(ui.streambox->currentIndex(), cmd + "\r\n");
		//}
		emit sgnWrite(cmd.toLocal8Bit() + "\r\n");
		emit sgnAddHistory(cmd);
	}
}

void ControlWidget::onAddCmd(const QString cmd)
{
	ui.send_txt->appendPlainText(cmd);
	this->activateWindow();
}

void ControlWidget::onClearClicked()
{
	ui.color_msg_txt->clear();
	m_MsgLines = 0;
}

void ControlWidget::onShowDataLog(QByteArray log)
{
	if (ui.pause_btn->isChecked() == false) {//��Ϊ��ɫ�Ƿ���
		return;
	}
	if (m_MsgLines >= 1000) {
		onClearClicked();
	}
	static bool text_start = 0;
	// ����һ���µ���ɫ��ʽ
	QTextCharFormat format;
	format.setForeground(QColor(Font_Green));  // ��������ɫ����Ϊ��ɫ
	// ����һ���µ��ı����
	QTextCursor cursor(ui.color_msg_txt->document()->end());
	cursor.movePosition(QTextCursor::End);
	if (log.startsWith("$") || log.startsWith("#")){
		if (log.startsWith("$Error")) {
			format.setForeground(QColor(Font_Red));  // ��������ɫ����Ϊ��ɫ
		}
		cursor.insertText("Device >: " + log, format);
		if (log.endsWith("\r\n")) {//�ַ�����β
			text_start = 0;//�ַ�����������ƴ��
		}
		else {
			text_start = 1;//����ƴ���ַ���
		}
	}
	else if(text_start == 1) {//ƴ���ַ���
		cursor.insertText(log, format);
		if (log.endsWith("\r\n")) {//�ַ�����β
			text_start = 0;//�ַ�����������ƴ��
		}
	}		
	ui.color_msg_txt->moveCursor(QTextCursor::End);
	m_MsgLines++;
}

void ControlWidget::onShowMsg(int level, const QString log)
{
	// ����һ���µ���ɫ��ʽ
	QTextCharFormat format;
	if (level == MSG_INFO) {
		format.setForeground(QColor(Font_Blue));  // ��������ɫ����Ϊ��ɫ
	}
	else if (level == MSG_WARNING) {
		format.setForeground(QColor(Font_Oragne));  // ��������ɫ����Ϊ��ɫ
	}
	else if (level == MSG_ERROR) {
		format.setForeground(QColor(Font_Red));  // ��������ɫ����Ϊ��ɫ
	}
	// ����һ���µ��ı����
	QTextCursor cursor(ui.color_msg_txt->document()->end());
	cursor.movePosition(QTextCursor::End);
	cursor.movePosition(QTextCursor::End);
	cursor.insertText("System >: " + log + "\r\n", format);
	ui.color_msg_txt->moveCursor(QTextCursor::End);
}

void ControlWidget::onChangeAutoSend()
{
	if (ui.AutoSend->isChecked()) {
		ui.send_txt->setEnabled(false);
		ui.clear_send->setEnabled(false);
		ui.Interval->setEnabled(false);
		ui.IntervalLable->setEnabled(false);
		m_AutoSendTimer.start(ui.Interval->value() * 1000);
		connect(&m_AutoSendTimer, &QTimer::timeout, this, &ControlWidget::onAutoSend);
	}
	else {
		ui.send_txt->setEnabled(true);
		ui.clear_send->setEnabled(true);
		ui.Interval->setEnabled(true);
		ui.IntervalLable->setEnabled(true);
		m_AutoSendTimer.stop();
		disconnect(&m_AutoSendTimer, &QTimer::timeout, this, &ControlWidget::onAutoSend);
	}
}

void ControlWidget::onPause(bool checked) {
	if (checked) {
		ui.pause_btn->setText(QChar(0xf04c));//pause
	}
	else {
		ui.pause_btn->setText(QChar(0xf04b));//go
	}
}

void ControlWidget::onAutoSend(){
	QString content = ui.send_txt->toPlainText();
	if (content.trimmed().isEmpty()) {
		return;
	}
	onSendCmd(content);
}