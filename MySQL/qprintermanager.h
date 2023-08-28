/*
 * feat:打印查询的所有内容(表格)
 * todo :
 *  打印单行的内容(文字)
 */

#ifndef QPRINTERMANAGER_H
#define QPRINTERMANAGER_H

#include <QObject>
#include <QAbstractItemModel>
#include <QTextStream>
#include <QPrinter>
#include <QDebug>
#include <QTextEdit>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QTextDocument>
#include <QDateTime>

class QPrinterManager : public QObject
{
    Q_OBJECT

public:
    explicit QPrinterManager(QObject *parent = nullptr);
    ~QPrinterManager(){}
    void setCharSet(const QString &set = QString("utf-8")){ m_charSet = set; }
    void dataBegin();
    void dataEnd(){ out << QString("</html>"); }
    // 添加主标题（两行：第一行居中显示文字，第二行居右显示时间）
    void insertTitle(const QString &title);
    // 添加次级标题
    void insertTitle2(const QString &title2);
    // 空出一行
    void insertSeperator(){ out << QString("<br>\n"); }
    // 不支持嵌套TABLE
    void tableBegin(const QStringList &head);
    void tableEnd(){ out << QString("</table></div>\n"); }
    void insert2TableRow(const QStringList &rowData);
    // 带预览
    void printWithPreview();
    // 直接打印，不带预览
    void printDirect();
    //打印一行数据即打印textBrowser中输出的行信息
    void printOneRowWithPreview();

    void insert2OneRow(const QStringList &oneRowData);


private:
    void resetState();

private slots:
    void getPreviewData(QPrinter *printer);


private:
    QString m_charSet;
    QString m_data;
    QTextStream out;
};

#endif // QPRINTERMANAGER_H
