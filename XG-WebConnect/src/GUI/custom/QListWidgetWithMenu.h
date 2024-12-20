#pragma once

#include <QApplication>
#include <QListWidget>
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>

class QListWidgetWithMenu  : public QListWidget
{
	Q_OBJECT

public:
	QListWidgetWithMenu(QWidget* parent = nullptr);
	~QListWidgetWithMenu();
private slots:
	void deleteItem();
protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
private:
	QMenu* menu;
	QAction* deleteAction;
};
