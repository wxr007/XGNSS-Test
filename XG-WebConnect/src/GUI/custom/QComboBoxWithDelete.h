#ifndef COMBOVIEW_H
#define COMBOVIEW_H

#include <QDebug>
#include <QStyle>
#include <QLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListView>
#include <QScrollBar>
#include <QListWidget>
#include <QPushButton>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QJsonObject>

class ComboView : public QListWidget
{
    Q_OBJECT
public:
    explicit ComboView(QWidget* parent = nullptr);
    QRect visualRect(const QModelIndex& index) const override;
};

class ComboItem : public QWidget
{
    Q_OBJECT
public:
    explicit ComboItem(const QString& text, QWidget* parent = nullptr);
    QString text() const;
signals:
    void itemClicked(const QString& text);
private:
    QString textValue;
    QPushButton* btn;
};

class ComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
};

class QComboBoxWithDelete : public QComboBox
{
    Q_OBJECT
public:
    explicit QComboBoxWithDelete(QWidget* parent = nullptr);
    //设置下拉选项
    void setRemovableItems(const QStringList& items);
    //显示弹框
    void showPopup() override;
    void hidePopup() override;
    void MyAddItem(const QString& text);
    void addCurruntText(QString text);
    //保存到Json和从Json加载
    void saveJson(QJsonObject& json);
    void loadJson(QJsonObject& json);
signals:
    void itemChange();
protected:
    void keyReleaseEvent(QKeyEvent* event);
private:
    QListWidget* itemList;
};

#endif // COMBOVIEW_H