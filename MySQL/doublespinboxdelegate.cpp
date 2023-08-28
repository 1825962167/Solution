/*
 * 修改qtableview控件委任：
 * 1、spinbox 精度到小数点后七位
 * 2、datetime 编辑格式变成 yyyy-MM-dd HH:mm:ss
 * 3、date 编辑格式变成 yyyy-MM-dd
 * 4、time 编辑格式变成 HH:mm:ss
 * 5、datetime 在qtableview中显示格式 yyyy-MM-dd HH:mm:ss
 */

#include "doublespinboxdelegate.h"
#include <QDebug>


DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* DoubleSpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QWidget *editor = new QWidget(parent);
    if (QVariant::Double == index.model()->data(index, Qt::EditRole).type())
    {
        QDoubleSpinBox *pEditor = new QDoubleSpinBox(parent);
        pEditor->setFrame(false);
        pEditor->setMinimum(-180.0000000);
        pEditor->setMaximum(180.0000000);
        pEditor->setDecimals(7);
        return pEditor;
    }
    else if(QVariant::DateTime == index.model()->data(index,Qt::EditRole).type())
    {
        QDateTimeEdit *dateTime = new QDateTimeEdit(parent);
        dateTime->setCalendarPopup(true);
        dateTime->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
        dateTime->calendarWidget()->setLocale(QLocale::Chinese);
        editor = dateTime;
        return editor;
    }
    else if (QVariant::Date == index.model()->data(index,Qt::EditRole).type())
    {
        QDateEdit *date = new QDateEdit(parent);
        date->setCalendarPopup(true);
        date->setDisplayFormat("yyyy-MM-dd");
        date->calendarWidget()->setLocale(QLocale::Chinese);
        editor = date;
        return editor;
    }
    else if (QVariant::Time == index.model()->data(index,Qt::EditRole).type())
    {
        QTimeEdit *time = new QTimeEdit(parent);
        time->setDisplayFormat("HH:mm:ss");
        editor = time;
        return editor;
    }
    else
    {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}
void DoubleSpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QVariant::Double == index.model()->data(index, Qt::EditRole).type())
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        QDoubleSpinBox *pSpinBox = static_cast<QDoubleSpinBox*>(editor);
        pSpinBox->setValue(value);
    }
    else if(QVariant::DateTime == index.model()->data(index,Qt::EditRole).type())
    {
        QDateTimeEdit *dateTime = static_cast<QDateTimeEdit *>(editor);
//        auto value = index.model()->data(index, Qt::EditRole);
//        dateTime->setDateTime(QDateTime::fromString(value.toString(), "yyyy-MM-dd HH:mm:ss"));
        dateTime->setDateTime(index.data().toDateTime());

    }
    else if (QVariant::Date == index.model()->data(index,Qt::EditRole).type())
    {
        auto value = index.model()->data(index, Qt::EditRole);
        QDateEdit *date = static_cast<QDateEdit *>(editor);
        date->setDate(QDate::fromString(value.toString(), "yyyy-MM-dd"));
    }
    else if (QVariant::Time == index.model()->data(index,Qt::EditRole).type())
    {
        auto value = index.model()->data(index, Qt::EditRole);
        QTimeEdit *time = static_cast<QTimeEdit *>(editor);
        time->setTime(QTime::fromString(value.toString(), "HH:mm:ss"));
    }
    else
    {
         QStyledItemDelegate::setEditorData(editor, index);
    }
}
void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
    if (QVariant::Double == index.model()->data(index, Qt::EditRole).type())
    {
        QDoubleSpinBox *pSpinBox = static_cast<QDoubleSpinBox*>(editor);
        pSpinBox->interpretText();
        double value = pSpinBox->value();
        model->setData(index, value, Qt::EditRole);
    }
    else if(QVariant::DateTime == index.model()->data(index,Qt::EditRole).type())
    {
        QVariant value = model->data(index);
        QDateTimeEdit *dateTime = static_cast<QDateTimeEdit *>(editor);
        value = dateTime->dateTime().toString("yyyy-MM-dd HH:mm:ss");
        model->setData(index, value, Qt::EditRole);
    }
    else if (QVariant::Date == index.model()->data(index,Qt::EditRole).type())
    {
        QVariant value = model->data(index);
        QDateEdit *date = static_cast<QDateEdit *>(editor);
        value = date->date().toString("yyyy-MM-dd");
        model->setData(index, value, Qt::EditRole);
    }
    else if (QVariant::Time == index.model()->data(index,Qt::EditRole).type())
    {
        QVariant value = model->data(index);
        QTimeEdit *time = static_cast<QTimeEdit *>(editor);
        value = time->time().toString("HH:mm:ss");
        model->setData(index, value, Qt::EditRole);
    }
    else
    {
        QStyledItemDelegate::setModelData(editor, model,index);
    }
}
void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

QString DoubleSpinBoxDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.type() == QVariant::DateTime)
    {
        return value.toDateTime().toString("yyyy-MM-dd HH:mm:ss");

    }
    return QStyledItemDelegate::displayText(value,locale);
}
