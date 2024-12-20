#include "QListWidgetWithMenu.h"


QListWidgetWithMenu::QListWidgetWithMenu(QWidget* parent)
	: QListWidget(parent)
{
    // 创建一个菜单
    menu = new QMenu(this);
    // 创建一个 QAction 对象
    deleteAction = new QAction(tr("Delete"), this);
    // 为 QListWidget 添加一个 context menu，并将 QAction 对象添加到其中
    setContextMenuPolicy(Qt::ActionsContextMenu);
    addAction(deleteAction);
    menu->addAction(deleteAction);
    // 连接 QAction 的触发信号，以便在点击该选项时执行相应的操作
    connect(deleteAction, &QAction::triggered, this, &QListWidgetWithMenu::deleteItem);
}

QListWidgetWithMenu::~QListWidgetWithMenu()
{}

void QListWidgetWithMenu::contextMenuEvent(QContextMenuEvent * event)
{
    // 显示 context menu
    if (menu) {
        menu->exec(mapToGlobal(event->pos()));
    }
}

void QListWidgetWithMenu::deleteItem() {
    // 处理删除操作，例如从 QListWidget 中删除当前选定的项
    QListWidgetItem* currentItem = this->currentItem();
    if (currentItem) {
        delete currentItem;
        clearSelection();
    }
}