#include "QPrinterManager.h"


QPrinterManager::QPrinterManager(QObject *parent) : QObject(parent), m_charSet("utf-8")
{
    out.setString(&m_data);
}

void QPrinterManager::dataBegin()
{
    out << QString("<html>")
        << QString("<head>")
        << QString("<meta content=\"text/html; charset=%1\">").arg(m_charSet)
        << QString("<meta name=Generator content=\"Microsoft Word 12 (filtered)\">")
        << QString("</head>");
}

void QPrinterManager::insertTitle(const QString &title)
{
    out << QString("<p style='text-align:center'><span style='font-size:20.0pt;font-family:宋体'>%1</span></p>\n")
           .arg(title)
        << QString("<p style='text-align:right'><span style='font-size:12.0pt;font-family:宋体'>打印时间：%1</span></p>\n")
           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
}

void QPrinterManager::insertTitle2(const QString &title2)
{
    out << QString("<p style='text-align:left'><span style='font-size:16.0pt;font-family:宋体'><b>-%1</b></span></p>\n")
           .arg(title2);
}
void QPrinterManager::tableBegin(const QStringList &head)
{
    out << QString("<div align='center' style='width:900px;margin-top:-100px'>")
        << QString("<table class=print width=100% border=1 cellspacing=0 cellpadding=5 style=border-collapse:collapse >\n")
        << QString("<thead><tr>\n");
    for (int i = 0; i < head.count(); ++i)
    {
        //qDebug() << "i = " << i;
        out << QString("<td nowrap='nowarp' style='border:solid windowtext 2.0pt' ><p align='center'><span align='center' style='font-size:16.0pt;font-family:宋体'><b>%1</b></span></p></td>\n")
               .arg(head.at(i));
    }
    out << "</tr></thead>\n";
}

void QPrinterManager::insert2TableRow(const QStringList &rowData)
{
    out << QString("<tr>\n");
    //qDebug() << rowData.count();
    //qDebug() << rowData;
    for (int i = 0; i < rowData.count(); ++i)
    {
        out << QString("<td><p align='center'><span align='center' style='font-size:10.0pt;font-family:宋体'>%1</span></p></td>\n")
               .arg(rowData.at(i));
    }
    out << QString("</tr>\n");
}

// 带预览 打印
void QPrinterManager::printWithPreview()
{
    QPrinter printer(QPrinter::ScreenResolution);
    //由于一列的数据项数过多，将打印预览页面设置为水平
    printer.setOrientation(QPrinter::Landscape);
    //设置页边距
    printer.setPageMargins(2, 2, 6, 6, QPrinter::Millimeter);
    printer.setPageSize(QPrinter::A4);
    QPrintPreviewDialog preview(&printer);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(getPreviewData(QPrinter*)));
    //preview.setWindowState(Qt::WindowNoState);
    preview.exec();
    resetState();

}

void QPrinterManager::insert2OneRow(const QStringList &oneRowData)
{
    for(int i = 0;i<oneRowData.count();++i)
    {
        out<< QString("<p align='left'><span align='left' style='font-size:10.0pt;font-family:宋体'>%1</span></p>\n")
              .arg(oneRowData.at(i));
    }
}

//只打印一行信息
void QPrinterManager::printOneRowWithPreview()
{
    QPrinter printer(QPrinter::ScreenResolution);
    //由于一列的数据项数过多，将打印预览页面设置为竖直
    printer.setOrientation(QPrinter::Portrait);
    //设置页边距
    printer.setPageMargins(2, 2, 6, 6, QPrinter::Millimeter);
    printer.setPageSize(QPrinter::A4);
    QPrintPreviewDialog preview(&printer);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(getPreviewData(QPrinter*)));
    //preview.setWindowState(Qt::WindowNoState);
    preview.exec();
    resetState();
}

// 直接打印，不带预览
void QPrinterManager::printDirect()
{
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setPageSize(QPrinter::A4);
    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted)
    {
        getPreviewData(&printer);
    }
    resetState();
}

void QPrinterManager::resetState()
{
    m_data.clear();
    m_charSet = "utf-8";
}

void QPrinterManager::getPreviewData(QPrinter *printer)
{
    QTextDocument document;
    document.setHtml(m_data);
    document.print(printer);
}


