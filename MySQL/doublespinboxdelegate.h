#ifndef DOUBLESPINBOXDELEGATE_H
#define DOUBLESPINBOXDELEGATE_H

#include <QWidget>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QDateTimeEdit>
#include <QCalendarWidget>

//小数点后七位，利用QSpinBox委托进行输入限制
class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DoubleSpinBoxDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QString displayText(const QVariant &value, const QLocale &locale) const override;
};


#endif // DOUBLESPINBOXDELEGATE_H
