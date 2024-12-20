#include "QListWidgetWithMenu.h"


QListWidgetWithMenu::QListWidgetWithMenu(QWidget* parent)
	: QListWidget(parent)
{
    // ����һ���˵�
    menu = new QMenu(this);
    // ����һ�� QAction ����
    deleteAction = new QAction(tr("Delete"), this);
    // Ϊ QListWidget ���һ�� context menu������ QAction ������ӵ�����
    setContextMenuPolicy(Qt::ActionsContextMenu);
    addAction(deleteAction);
    menu->addAction(deleteAction);
    // ���� QAction �Ĵ����źţ��Ա��ڵ����ѡ��ʱִ����Ӧ�Ĳ���
    connect(deleteAction, &QAction::triggered, this, &QListWidgetWithMenu::deleteItem);
}

QListWidgetWithMenu::~QListWidgetWithMenu()
{}

void QListWidgetWithMenu::contextMenuEvent(QContextMenuEvent * event)
{
    // ��ʾ context menu
    if (menu) {
        menu->exec(mapToGlobal(event->pos()));
    }
}

void QListWidgetWithMenu::deleteItem() {
    // ����ɾ������������� QListWidget ��ɾ����ǰѡ������
    QListWidgetItem* currentItem = this->currentItem();
    if (currentItem) {
        delete currentItem;
        clearSelection();
    }
}