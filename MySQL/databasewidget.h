#ifndef DATABASEWIDGET_H
#define DATABASEWIDGET_H

#include <QWidget>
#include<QSqlTableModel>
#include "database.h"
#include <QSqlRecord>
#include <QMessageBox>
#include "qprintermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include <QtMath>
#include <QSqlResult>
#include <QTimer>
#include "doublespinboxdelegate.h"
#include <QDateTime>
#include <QFileDialog>
#include <QStandardPaths>
#include <QAxObject>
#include <QDesktopServices>
#include <qmath.h>
#include <QSqlIndex>
#include <QCalendarWidget>
#include <QResource>
#include <QInputDialog>
#include <QSqlField>
#include <QMenu>
#include <QVariant>


namespace Ui {
class DatabaseWidget;
}

class DatabaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatabaseWidget(QWidget *parent = nullptr);
    ~DatabaseWidget();
    QSqlTableModel *model;

protected:


private:
    void tableViewInit_page();

    void printLog(const QString &log);

    void updatePageCount(int count);

    bool jump(int page);

    int queryRowCount();

signals:
    void singleClicked();
//    void doubleClicked();

private slots:
    void on_tableView_clicked(const QModelIndex &index);

    void on_deleteButton_clicked();

    void on_saveButton_clicked();

    void on_selectButton_clicked();

    void on_addButton_clicked();

    void set_tableView_model();

    void on_refreshButton_clicked();

    void export_tableView();

    void print_tabelView();

    void on_firstButton_clicked();

    void on_previousButton_clicked();

    void on_nextButton_clicked();

    void on_lastButton_clicked();

    void on_previousPage_returnPressed();

    void singleClicked_slot();

    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_HeaderClicked(int index);

    void on_HeaderDoubleClicked(int index);

    void on_displayRowCount_editingFinished();

    void showContextMenu(const QPoint &point);

    void on_printButton_clicked();

private:
    Database myDatabase;
    Ui::DatabaseWidget *ui;
    //显示图片所需属性
    QTimer *m_clickedTimer;

    //分页属性
    int mCurrentPage=0;                               //0开始索引
    int mTotalPage;                                   //总页数
    int mOnepageCount=30;                             //界面显示行数限制
    QString mTableName = "result";
    QString sql = "";                                 //sql语句不包含limit
    //排序
    QString sortOrderSql = QString(" order by ");      //默认升序排序
    QString fieldName = QString(" id ");           //默认是在result表里按照id(主键)查询
    QString sortMethod = QString(" asc ");
    Qt::SortOrder sortOrder = Qt::AscendingOrder;     //当前的排序方法
    //时间段查询
    QString timeStampSql = QString(" 测试时间 between ");
    QString timeStampSqlAnd = QString( " and ");
    QString timeStampL;
    QString timeStampR;
    QDateTime defaultDatetimeL = QDateTime(QDate(2000,1,1),QTime(0,0,0));
    QDateTime defaultDatetimeR = QDateTime(QDate(2023,10,1),QTime(0,0,0));



};

#endif // DATABASEWIDGET_H
