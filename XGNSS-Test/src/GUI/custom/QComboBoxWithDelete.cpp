#include "QComboBoxWithDelete.h"
#include <QFontDataBase>
#include <QJsonArray>

ComboView::ComboView(QWidget* parent)
    : QListWidget(parent)
{

}

QRect ComboView::visualRect(const QModelIndex& index) const
{
    QRect rect = QListWidget::visualRect(index);
    int width = this->width();
    if (verticalScrollBar()->isVisible()) {
        width -= verticalScrollBar()->width();
    }
    rect.setWidth(width);
    return rect;
}

ComboItem::ComboItem(const QString& text, QWidget* parent)
    : QWidget(parent),
    textValue(text)
{
    //引入图形字体
    int fontId = QFontDatabase::addApplicationFont(":/XGNSSTest/res/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont iconFont = QFont(fontName);

    btn = new QPushButton(this);
    btn->setFixedSize(18, 18);
    btn->setFont(iconFont);
    btn->setText(QChar(0xf00d));

    btn->setStyleSheet("QPushButton{background-color: transparent; padding:0; border: none; color: rgb(64, 158, 255);}"
        "QPushButton:hover{color: rgb(245, 108, 108);}");
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addStretch();
    layout->addWidget(btn);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(0);
    this->setLayout(layout);
    connect(btn, &QPushButton::clicked, [this] {
        emit itemClicked(textValue);
        });
}

QString ComboItem::text() const
{
    return textValue;
}

ComboDelegate::ComboDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void ComboDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    //去掉焦点带来的虚线框
    opt.state &= ~QStyle::State_HasFocus;

    initStyleOption(&opt, index);
    QStyledItemDelegate::paint(painter, opt, index);
}

#define SAVED_CUR_NAME      "text"
#define SAVED_LIST_NAME     "list"

QComboBoxWithDelete::QComboBoxWithDelete(QWidget* parent)
    : QComboBox(parent),
    itemList(new ComboView(this))
{
    //    itemList->setTextElideMode(Qt::ElideNone);
    setModel(itemList->model());
    setView(itemList);
    setEditable(true);
    setItemDelegate(new ComboDelegate(this));
    setMaxVisibleItems(5);
}

void QComboBoxWithDelete::setRemovableItems(const QStringList& items)
{
    //combox的additem insertitem不是虚函数
    //实现里时调用的model->insertRow，但是懒得再去重写listmodel-view，就新增一个接口
    itemList->clear();
    if (items.isEmpty())
        return;
    for (int i = 0; i < items.count(); i++) {
        MyAddItem(items.at(i));
    }
}

void QComboBoxWithDelete::MyAddItem(const QString& text)
{
    QListWidgetItem* widget_item = new QListWidgetItem(itemList);
    ComboItem* item = new ComboItem(text, itemList);
    widget_item->setData(Qt::DisplayRole, text);
    itemList->setItemWidget(widget_item, item);
    connect(item, &ComboItem::itemClicked, this, [this, item, widget_item]() {
        //take移除item后没有刷新弹框大小，干脆隐藏掉先
        hidePopup();
        itemList->takeItem(itemList->row(widget_item));
        delete widget_item;
        if (itemList->count() <= 5) {
            for (int i = 0; i < itemList->count(); i++) {
                QWidget* nwidget = itemList->itemWidget(itemList->item(i));
                QHBoxLayout* layout = (QHBoxLayout*)nwidget->layout();
                layout->setContentsMargins(5, 0, 5, 0);
                nwidget->setLayout(layout);
            }
        }
        emit itemChange();
        });
    if (itemList->count() > 5) {
        for (int i = 0; i < itemList->count(); i++) {
            QWidget* nwidget = itemList->itemWidget(itemList->item(i));
            QHBoxLayout* layout = (QHBoxLayout*)nwidget->layout();
            layout->setContentsMargins(5, 0, 15, 0);
            nwidget->setLayout(layout);
        }
    }
}

void QComboBoxWithDelete::addCurruntText(QString text)
{
    for (int i = 0; i < count(); ++i) {
        ComboItem* m_widget = qobject_cast<ComboItem*>(itemList->itemWidget(itemList->item(i)));
        if (!m_widget) removeItem(i);
    }
    for (int i = 0; i < count(); ++i) {
        if (itemText(i) == text) return;
    }
    MyAddItem(text);
}

void QComboBoxWithDelete::saveJson(QJsonObject& json)
{
    QString current =currentText().trimmed();
    QJsonArray array;
    if (!current.isEmpty()) {
        json.insert(SAVED_CUR_NAME, current);
        if (!array.contains(current)) {
            array.append(current);
        }
    }
    for (int i = 0; i < count(); ++i) {
        QString item = itemText(i).trimmed();
        if (!item.isEmpty() && !array.contains(item)) {
            array.append(item);
        }
    }
    json.insert(SAVED_LIST_NAME, array);
}

void QComboBoxWithDelete::loadJson(QJsonObject& json)
{
    if (json.isEmpty())
        return;
    if (json[SAVED_CUR_NAME].isString()) {
        setCurrentText(json[SAVED_CUR_NAME].toString());
    }
    if (json[SAVED_LIST_NAME].isArray()) {
        QJsonArray array = json[SAVED_LIST_NAME].toArray();
        for (int i = 0; i < array.size(); i++) {
            if (array[i].toString().trimmed().isEmpty()) {
                continue;
            }
            if (array[i].isString()) {
                MyAddItem(array[i].toString());
            }
        }
    }
}

void QComboBoxWithDelete::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        QString text = currentText();
        addCurruntText(text);
        setCurrentText(text);
    }
    else {
        QComboBox::keyPressEvent(event);
    }
}

void QComboBoxWithDelete::showPopup()
{
    QComboBox::showPopup();
}

void QComboBoxWithDelete::hidePopup()
{
    QStyle* const style = this->style();
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    view()->scrollTo(view()->currentIndex(),
        style->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
        ? QAbstractItemView::PositionAtCenter
        : QAbstractItemView::EnsureVisible);
    QComboBox::hidePopup();
}

