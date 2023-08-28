#include "databasewidget.h"
#include "ui_databasewidget.h"


/*
 *todo :
 *  1.保存 如果增加数据 判断增加的数据是否符合保存要求  默认条件下是不会有提示的 如果不符合不会保存且不会有提示 got
 *  2.刷新功能  got
 *         //置空过滤器 刷新table
            ui->lineEdit->clear();
            model->setFilter(NULL);
            model->select();
     3.撤回功能 无法完成

 * */
DatabaseWidget::DatabaseWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DatabaseWidget)
{
    ui->setupUi(this);
    //全屏显示界面
    showMaximized();
    this->myDatabase.getConnection();
    model = new QSqlTableModel;
    model->setTable("result");
    ui->tableView->setModel(model);
    //设置tableview的样式
    QFile file(":/MySQL/tableViewStyleSheet.qss");
    if(file.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(file.readAll());
        ui->tableView->setStyleSheet(styleSheet);
        file.close();
    }
    //设置table的标题栏
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//列宽随窗口大小改变而改变，每列平均分配，充满整个表，但是此时列宽不能拖动进行改变
//    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);  //用户可拉伸
    ui->tableView->horizontalHeader()->setHighlightSections(false);   //设置水平标题栏禁止高亮
    ui->tableView->setAlternatingRowColors(true);
    //table初始化
    //时间段查询
    this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
    this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
    tableViewInit_page();
    model->select();
    ui->displayRowCount->setText(QString::number(this->mOnepageCount));
    //设置model的编辑模式，手动提交修改
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);//修改后不会自动保存
    this->ui->textBrowser->setText(QString("欢迎您，使用本公司产品！\n"));

    //使该窗口close()后自动销毁
    setAttribute(Qt::WA_DeleteOnClose, true);
    //一次选中一行  暂时不用
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //  设置doublespin保留小数点的位数为7位  datetime的输入格式为yyyy-MM-dd HH:mm:ss
    ui->tableView->setItemDelegate(new DoubleSpinBoxDelegate(this));
    setWindowTitle("数据库");

    //图片显示初始化
    this->m_clickedTimer = new QTimer(this);
    this->m_clickedTimer->setSingleShot(true);//设置定时器单次触发
    connect(this->m_clickedTimer,SIGNAL(timeout()),this,SLOT(singleClicked_slot()));

    //排序
    QHeaderView *headerView = ui->tableView->horizontalHeader();
    connect(headerView,SIGNAL(sectionClicked(int)),this,SLOT(on_HeaderClicked(int)));

    ui->tableView->setSortingEnabled(false);
    ui->tableView->horizontalHeader()->setSortIndicator(0,Qt::AscendingOrder);
    ui->tableView->horizontalHeader()->setSortIndicatorShown(true);

    headerView->setSectionsMovable(true);
    headerView->setSectionsClickable(true);
//    headerView->setSectionResizeMode();
    headerView->setEditTriggers(QHeaderView::DoubleClicked | QHeaderView::AnyKeyPressed);
    connect(headerView,SIGNAL(sectionDoubleClicked(int)),this,SLOT(on_HeaderDoubleClicked(int)));
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(headerView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));


}


void DatabaseWidget::singleClicked_slot()
{
    QModelIndex index = ui->tableView->currentIndex();
    if(index.column() == 28 )
    {
        if(!index.data().toString().isEmpty())
        {
            QString fileName = index.data().toString();
            QString save_path = QDir("./assets").path()+"/"+fileName;
            QString abso_path = QDir::toNativeSeparators(QDir(save_path).absolutePath());
            QString urlPath = QUrl::fromLocalFile(abso_path).toString();
            if(urlPath.isEmpty())return;
            QDesktopServices::openUrl(urlPath);
        }
    }
}
//鼠标点到table，触发将鼠标所在行输出到listwidget中
void DatabaseWidget::on_tableView_clicked(const QModelIndex &index)
{
    ui->textBrowser->clear();
    QSqlRecord record = model->record(index.row());

    int col = record.count();
    for(int i = 0; i<col;++i)
    {
        QVariant colFieldName = model->headerData(i,Qt::Horizontal);
        QString fieldNamestr = colFieldName.toString();
        QVariant cellType = record.value(fieldNamestr).type();
        if(cellType == QVariant::DateTime)
        {
            ui->textBrowser->append(QString("%1:%2").arg(fieldNamestr).arg(record.value(fieldNamestr).toDateTime().toString("yyyy-MM-dd HH:mm:ss")));
        }
        else
        {
            ui->textBrowser->append(QString("%1:%2").arg(fieldNamestr).arg(record.value(fieldNamestr).toString()));
        }
    }
    //图片显示
    if(index.column() == 28)
    {
        this->m_clickedTimer->start(50);
    }


}

void DatabaseWidget::on_tableView_doubleClicked(const QModelIndex &index)
{
    if(index.column() == 28)
    {
        this->m_clickedTimer->stop();
    }

}

void DatabaseWidget::on_deleteButton_clicked()
{
    QItemSelectionModel *m = ui->tableView->selectionModel();
    QModelIndexList list = m->selectedRows();
    if(list.size() == 0) return ;
    int isDelete = QMessageBox::warning(this,tr("删除所选行"),tr("确定删除所选行吗？"),QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
    if(isDelete == QMessageBox::No) return;
    //删除所有选中的行
    for(int i=0;i<list.size();i++){
        model->removeRow(list.at(i).row());
    }
//    on_saveButton_clicked();
}

void DatabaseWidget::on_saveButton_clicked()
{
    if(!model->submitAll())
    {          
        QMessageBox::warning(nullptr,"警告","保存失败!\n不可添加空白行\n产品编号、测试时间、操作员编号不能为空",QMessageBox::Yes);
    }
    qDebug()<<"[database]保存成功";
}

void DatabaseWidget::on_selectButton_clicked()
{
    int index = this->ui->comboBox->currentIndex();
    QString lineEdit = this->ui->lineEdit->text();
    mCurrentPage = 0;
    int colCount = ui->tableView->horizontalHeader()->count();
    switch(index)
    {
        case 0:
//            QMessageBox::information(NULL,"提示","请选择查询方式!",QMessageBox::Ok);
            ui->lineEdit->clear();
            sql = "";
            if(this->mTableName == "result")
            {
                this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            }
            tableViewInit_page();
            model->select();
            break;
        case 1:
            if(mTableName=="result")
            {
                sql = QString(" 产品编号 like '%%1%'").arg(lineEdit);
                this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                tableViewInit_page();
                model->select();
            }
            else
            {
                sql = QString("用户名 like '%%1%'").arg(lineEdit);
                tableViewInit_page();
                model->select();
            }
            break;
        case 2:
            if(this->mTableName == "result")
            {
                this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            }
            sql = QString("操作员编号 like '%%1%'").arg(lineEdit);
            tableViewInit_page();
            model->select();
            break;
        case 3:
            if(this->mTableName == "result")
            {
                this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            }
            sql = QString("测试时间 like '%%1%'").arg(lineEdit);
            tableViewInit_page();
            model->select();
            break;
        case 4:
            if(this->mTableName == "result")
            {
                this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
                this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            }
//            sql = QString(" concat(IFNULL(`%1`,''),IFNULL(`%2`,''),IFNULL(`%3`,''),IFNULL(`%4`,''),"
//                           "IFNULL(`%5`,''),IFNULL(`%6`,''),IFNULL(`%7`,''),IFNULL(`%8`,''),"
//                           "IFNULL(`%9`,''),IFNULL(`%10`,''),IFNULL(`%11`,''),IFNULL(`%12`,''),"
//                           "IFNULL(`%13`,''),IFNULL(`%14`,''),IFNULL(`%15`,''),IFNULL(`%16`,''),"
//                           "IFNULL(`%17`,''),IFNULL(`%18`,''),IFNULL(`%19`,''),IFNULL(`%20`,''),"
//                           "IFNULL(`%21`,''),IFNULL(`%22`,''),IFNULL(`%23`,''),IFNULL(`%24`,''),"
//                           "IFNULL(`%25`,''),IFNULL(`%26`,''),IFNULL(`%27`,''),IFNULL(`%28`,''),"
//                           "IFNULL(`%29`,'')) like '%%30%'").arg(model->headerData(0,Qt::Horizontal).toString()).arg(model->headerData(1,Qt::Horizontal).toString()).arg(model->headerData(2,Qt::Horizontal).toString()).arg(model->headerData(3,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(4,Qt::Horizontal).toString()).arg(model->headerData(5,Qt::Horizontal).toString()).arg(model->headerData(6,Qt::Horizontal).toString()).arg(model->headerData(7,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(8,Qt::Horizontal).toString()).arg(model->headerData(9,Qt::Horizontal).toString()).arg(model->headerData(10,Qt::Horizontal).toString()).arg(model->headerData(11,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(12,Qt::Horizontal).toString()).arg(model->headerData(13,Qt::Horizontal).toString()).arg(model->headerData(14,Qt::Horizontal).toString()).arg(model->headerData(15,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(16,Qt::Horizontal).toString()).arg(model->headerData(17,Qt::Horizontal).toString()).arg(model->headerData(18,Qt::Horizontal).toString()).arg(model->headerData(19,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(20,Qt::Horizontal).toString()).arg(model->headerData(21,Qt::Horizontal).toString()).arg(model->headerData(22,Qt::Horizontal).toString()).arg(model->headerData(23,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(24,Qt::Horizontal).toString()).arg(model->headerData(25,Qt::Horizontal).toString()).arg(model->headerData(26,Qt::Horizontal).toString()).arg(model->headerData(27,Qt::Horizontal).toString())
//                                                          .arg(model->headerData(28,Qt::Horizontal).toString()).arg(lineEdit);
            QString field_name;
            for(int i = 0;i<colCount;++i)
            {
                field_name = model->headerData(i,Qt::Horizontal).toString();
                if(i == 0)
                {
                    sql = QString(" concat(IFNULL(`%1`,'')").arg(field_name);
                }
                else
                    sql += QString(",IFNULL(`%1`,'')").arg(field_name);
            }
            sql += QString(") like '%%1%'").arg(lineEdit);
            tableViewInit_page();
            model->select();
            break;

    }
}

void DatabaseWidget::on_addButton_clicked()
{
    //添加空记录
    QSqlRecord record = model->record(); //获取空记录
    //获取行号
    int row = model->rowCount();
    model->insertRecord(row, record);
}

void DatabaseWidget::set_tableView_model()
{
    mTableName = model->tableName();
    //根据table更新查询方式
    if(mTableName != "result")
    {
        ui->comboBox->clear();
        ui->comboBox->addItem("请选择查询方式");
        ui->comboBox->addItem("按用户名查询");
        ui->comboBox->addItem("按文字查询");
    }
    else
    {
        ui->comboBox->clear();
        ui->comboBox->addItem("请选择查询方式");
        ui->comboBox->addItem("按产品编号查询");
        ui->comboBox->addItem("按操作员编号查询");
        ui->comboBox->addItem("按测试时间查询");
        ui->comboBox->addItem("按文字查询");
    }

    on_refreshButton_clicked();
}

void DatabaseWidget::on_refreshButton_clicked()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
        }
    }
    //界面刷新
    ui->tableView->horizontalHeader()->setSortIndicator(0,Qt::AscendingOrder);
    ui->lineEdit->clear();
    ui->comboBox->setCurrentIndex(0);
    ui->textBrowser->clear();
    ui->dateTimeEdit->setDateTime(this->defaultDatetimeL);
    ui->dateTimeEdit1->setDateTime(this->defaultDatetimeR);
    mCurrentPage = 0;
    sql = "";
    sortMethod = QString(" asc ");
    this->fieldName = model->headerData(0,Qt::Horizontal).toString();
    sortOrder = Qt::AscendingOrder;
    this->timeStampL = QString(" '%1' ").arg(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
    this->timeStampR = QString(" '%1' ").arg(ui->dateTimeEdit1->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
    tableViewInit_page();
    model->select();
}

void DatabaseWidget::export_tableView()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            on_saveButton_clicked();
        }
    }
    ui->textBrowser->append("<font size=\"8\" color=\"red\">导出需要时间，请耐心等待哦！</font>");
    //查询结果暂时不按照分页显示
    model->setFilter(sql);
    model->select();
    //excel文件的保存格式不要是xls 软件是32位的 windows是64位会有兼容问题
    QString fileName = QFileDialog::getSaveFileName(this, QStringLiteral("保存"),
                                            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QStringLiteral("Excel 文件(*.xlsx)"));
    if (fileName != "")
    {
        QAxObject *excel = new QAxObject;
        if (excel->setControl("Excel.Application")) //连接Excel控件
        {
        excel->dynamicCall("SetVisible (bool Visible)", "false");//不显示窗体
        excel->setProperty("DisplayAlerts", false);//不显示任何警告信息
        QAxObject *workbooks = excel->querySubObject("WorkBooks");//获取工作簿集合
        workbooks->dynamicCall("Add");//新建一个工作簿
        QAxObject *workbook = excel->querySubObject("ActiveWorkBook");//获取当前工作簿
        QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);

        int i, j;
        int colount = model->columnCount();

        int rowcount = model->rowCount();

        QAxObject *cell, *col;
        //字段行
        for (i = 0; i<colount; i++)
        {
            QString columnName;
            if(i>=26){
                columnName.append(QChar(i/26-1+'A'));
                columnName.append(QChar(i%26+'A'));
                columnName.append(":");
                columnName.append(QChar(i/26-1+'A'));
                columnName.append(QChar(i%26+'A'));
            }
            else
            {
                columnName.append(QChar(i + 'A'));
                columnName.append(":");
                columnName.append(QChar(i + 'A'));
            }
            col = worksheet->querySubObject("Columns(const QString&)", columnName);
            col->setProperty("ColumnWidth", ui->tableView->columnWidth(i) / 6);
            cell = worksheet->querySubObject("Cells(int,int)", 1, i + 1);

            //QTableView 获取表格头部文字信息
            columnName= model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
            cell->dynamicCall("SetValue(const QString&)", columnName);
            cell->querySubObject("Font")->setProperty("Bold", true);
            cell->querySubObject("Interior")->setProperty("Color", QColor(191, 191, 191));
            cell->setProperty("HorizontalAlignment", -4108);//xlCenter
            cell->setProperty("VerticalAlignment", -4108);//xlCenter
        }
        //QTableView 获取表格数据部分
        for(i=0;i<rowcount;i++)
        {
            for (j=0;j<colount;j++)
            {
                QString strdata;
                QSqlRecord record = model->record(i);
                if(record.value(j).type()==QVariant::DateTime)
                {
                    strdata= record.value(j).toDateTime().toString("yyyy-MM-dd hh:mm:ss");
                }
                else
                {
                    QModelIndex index = model->index(i, j);
                    strdata= model->data(index).toString();
                }

                worksheet->querySubObject("Cells(int,int)", i+2, j+1)->dynamicCall("SetValue(const QString&)", strdata);
            }
        }
        //画框线
        QString lrange;
        lrange.append("A1:");
        if(colount>26)
        {
            lrange.append((colount-1)/26 - 1 + 'A');
            lrange.append((colount-1)%26 + 'A');
        }
        else
        {
            lrange.append(colount - 1 + 'A');
        }
        lrange.append(QString::number(model->rowCount() + 1));
        QAxObject * range = worksheet->querySubObject("Range(const QString&)", lrange);
        range->querySubObject("Borders")->setProperty("LineStyle", QString::number(1));
        range->querySubObject("Borders")->setProperty("Color", QColor(0, 0, 0));
        //调整数据区行高
        QString rowsName;
        rowsName.append("1:");
        rowsName.append(QString::number(model->rowCount() + 1));
        range = worksheet->querySubObject("Range(const QString&)", rowsName);
        range->setProperty("RowHeight", 20);
        workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(fileName));//保存至fileName
        workbook->dynamicCall("Close()");//关闭工作簿
        excel->dynamicCall("Quit()");//关闭excel
        delete excel;
        excel = NULL;

        if (QMessageBox::question(this, QStringLiteral("完成"), QStringLiteral("文件已经导出，是否现在打开？"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(fileName)));
        }
        else
            QMessageBox::warning(NULL, QStringLiteral("错误"), QStringLiteral("未能创建 Excel 对象，请安装 Microsoft Excel。"), QMessageBox::Apply);
    }
    //导出结束后 分页显示查询结果
    qDebug()<<"[database] 导出成功";
    on_refreshButton_clicked();


}

//打印只要sql条件的所有数据
void DatabaseWidget::print_tabelView()
{
    ui->textBrowser->append("<font size=\"8\" color=\"red\">打印需要时间，请耐心等待哦！</font>");
    //查询结果暂时不按照分页显示
    model->setFilter(sql);
    model->select();
    int colNum = model->columnCount();
    int rowNum = model->rowCount();
    //打印表格的标题
    QString title = QString("Information");
    //columnList为各列名
    QStringList columnList;
    //dataLists为各行数据
    QVector<QStringList> dataLists;
    for(int i = 0;i<colNum;++i)
    {
        QString fieldTab = model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
        columnList.append(fieldTab);
    }
    //表格内存储到dataLists
    for(int i=0;i<rowNum;i++)
    {
        QStringList data;
        for(int j=0;j<colNum;j++)
        {
            QString strdata;
            QSqlRecord record = model->record(i);
            if(record.value(j).type()==QVariant::DateTime)
            {
                strdata= record.value(j).toDateTime().toString("yyyy-MM-dd hh:mm:ss");
            }
            else
            {
                QModelIndex index = model->index(i, j);
                strdata= model->data(index).toString();
            }
            data<<strdata;

        }
        dataLists.push_back(data);
    }

    //声明打印（内容）QPrinterManager类，并调用其函数
    QPrinterManager PM;
    PM.dataBegin();
    PM.insertTitle(title);
//    PM.insertTitle2(tr("次级标题"));
    PM.tableBegin(columnList);
    for(int row=0;row<rowNum;row++)
    {
        PM.insert2TableRow(dataLists[row]);
    }

    PM.tableEnd();
    PM.dataEnd();
    PM.printWithPreview();
    //导出结束后 分页显示查询结果
    qDebug()<<"[database] 打印当前表所有内容";
    on_refreshButton_clicked();

}

//打印单行数据
void DatabaseWidget::on_printButton_clicked()
{
    QString title = QString("information");
    QStringList oneRowData = ui->textBrowser->toPlainText().split("\n");
    qDebug()<<oneRowData;
    ui->textBrowser->append("<font size=\"8\" color=\"red\">打印需要时间，请耐心等待哦！</font>");
    QPrinterManager PM;
    PM.dataBegin();
    PM.insertTitle(title);
    PM.insert2OneRow(oneRowData);
    PM.dataEnd();
    PM.printOneRowWithPreview();
    qDebug()<<"[database] 打印单行成功";
}

void DatabaseWidget::tableViewInit_page()
{
    //属性初始化
    int count = queryRowCount();
//    printLog("总行数:"+QString::number(count));
    updatePageCount(count);
    jump(mCurrentPage);//limit 限制100在查询

}

void DatabaseWidget::printLog(const QString &log)
{
    qDebug()<<log;
}

void DatabaseWidget::updatePageCount(int count)
{
    int p = qCeil(float(count) / (float)mOnepageCount);
    ui->pageCount->setText(QString("/%1页").arg(p));
    mTotalPage = p;
}

bool DatabaseWidget::jump(int page)
{
    QString strfilter;
    if(this->mTableName == "result")
    {
        if(!sql.isEmpty())
        {
             strfilter = sql + timeStampSqlAnd + timeStampSql + timeStampL +timeStampSqlAnd + timeStampR + this->sortOrderSql + this->fieldName + this->sortMethod + QString(" limit %1,%2;").arg( page * mOnepageCount ).arg(mOnepageCount);
        }
        else
        {
            strfilter =  timeStampSql + timeStampL +timeStampSqlAnd + timeStampR+ this->sortOrderSql + this->fieldName + this->sortMethod +QString(" limit %1,%2;").arg( page * mOnepageCount ).arg(mOnepageCount);//没有条件的话需要加1=1
        }
    }
    else
    {
        if(!sql.isEmpty())
        {
             strfilter = sql + this->sortOrderSql + this->fieldName + this->sortMethod + QString(" limit %1,%2;").arg( page * mOnepageCount ).arg(mOnepageCount);
        }
        else
        {
            strfilter = QString(" 1=1 ")+ this->sortOrderSql + this->fieldName + this->sortMethod +QString(" limit %1,%2;").arg( page * mOnepageCount ).arg(mOnepageCount);//没有条件的话需要加1=1
        }
    }
    qDebug()<<"[database] filter:"<<strfilter;
    if(!model) return false;
    model->setFilter(strfilter);
    ui->previousPage->setText( QString::number(page+1));
    return true;
}

int DatabaseWidget::queryRowCount()
{
    QSqlQuery query;
    QString temp;  
    if(this->mTableName == "result")
    {
        if(!sql.isEmpty())
        {
            temp = QString("select count(*) from %1 where ").arg(mTableName)+sql + timeStampSqlAnd + timeStampSql +timeStampL +timeStampSqlAnd + timeStampR ;
        }
        else
        {
            temp = QString("select count(*) from %1 where ").arg(mTableName) + timeStampSql +timeStampL +timeStampSqlAnd + timeStampR;
        }
    }
    else
    {
        if(!sql.isEmpty())
        {
            temp = QString("select count(*) from %1 where ").arg(mTableName)+sql ;
        }
        else
        {
            temp = QString("select count(*) from %1  ").arg(mTableName);
        }
    }

    if(!query.exec(temp))
    {
        qDebug()<<"error sql:"<<temp;
        QMessageBox::warning(nullptr,"提示","查询数据库数据行数失败",QMessageBox::Yes);
    }
    while(query.next())
    {
        return query.value(0).toInt();
    }
    return 0;

}

void DatabaseWidget::on_firstButton_clicked()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
            tableViewInit_page();
            model->select();
        }
    }
    mCurrentPage = 0;
    jump(mCurrentPage);
}

void DatabaseWidget::on_previousButton_clicked()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
            tableViewInit_page();
            model->select();
        }
    }
    mCurrentPage -= 1;
    if(mCurrentPage < 0){
        QMessageBox::warning(nullptr,"提示","页码不能小于0",QMessageBox::Yes);
        mCurrentPage = 0;
    }
    jump(mCurrentPage);
}

void DatabaseWidget::on_nextButton_clicked()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
            tableViewInit_page();
            model->select();
        }
    }
    mCurrentPage += 1;
    if(mCurrentPage > mTotalPage-1){
        QMessageBox::warning(nullptr,"提示","页码超过限制",QMessageBox::Yes);
        mCurrentPage = mTotalPage-1;
    }
    jump(mCurrentPage);
}

void DatabaseWidget::on_lastButton_clicked()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
            tableViewInit_page();
            model->select();
        }
    }
    mCurrentPage = mTotalPage-1;
    jump(mCurrentPage);
}

void DatabaseWidget::on_previousPage_returnPressed()
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
            tableViewInit_page();
            model->select();
        }
    }
    QString input = ui->previousPage->text();
    if(input.isEmpty()) return;

    int toPage = input.toInt() -1 ;
    if(toPage < 0  || toPage >mTotalPage-1)
    {
        QMessageBox::warning(nullptr,"提示","输入错误",QMessageBox::Yes);
    }
    mCurrentPage = toPage;
    jump(toPage);

}

//排序
void DatabaseWidget::on_HeaderClicked(int index)
{
    if(index == 0 ||index == 1 || index == 28 || index == 29 || index == 30)
    {
        //保存修改值
        if(model->isDirty())
        {
            int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
            if(isSave == QMessageBox::Yes)
            {
                model->submitAll();
                tableViewInit_page();
                model->select();
            }
        }
       this->fieldName = model->headerData(index,Qt::Horizontal).toString();
       this->sortOrder =(sortOrder==Qt::AscendingOrder)?Qt::DescendingOrder:Qt::AscendingOrder;
       if(this->sortOrder == Qt::AscendingOrder)
       {
           this->sortMethod = QString(" asc ");
       }
       else
       {
           this->sortMethod = QString(" desc ");
       }
       ui->tableView->horizontalHeader()->setSortIndicator(index,this->sortOrder);
       tableViewInit_page();
       qDebug()<<"[database] 排序成功";
       model->select();
    }

}

void DatabaseWidget::on_HeaderDoubleClicked(int index)
{
    QString oldName = model->headerData(index,Qt::Horizontal).toString();
    QSqlField field = model->record().field(index);
    QVariant::Type data_type = field.type();
    if(data_type == QVariant::DateTime)   //不能对测试时间进行编辑 因为它单击排序且分页、查询的时候回使用到
    {
        QMessageBox::warning(nullptr,"error","不可编辑!",QMessageBox::Yes);
        return ;
    }
    QString data_typeStr ;
    int data_length = field.length();
    int data_precision = field.precision();
    bool isOK;
    QString newName =QInputDialog::getText(this,tr("Edit header name"),tr("Enter new name:")
                                           ,QLineEdit::Normal,model->headerData(index,Qt::Horizontal).toString(),&isOK);
    QString sqlstr;
    QSqlQuery query;
    //字段的数据类型不同的话sql会有点不同，而且语句要固定！！！
    if(data_type == QVariant::Double)
    {
        data_typeStr = QString("double");
        sqlstr = QString("alter table %1 change  `%2`  `%3` %4(%5,%6) ").arg(this->mTableName).arg(oldName).arg(newName)
                                                                            .arg(data_typeStr).arg(data_length).arg(data_precision);        
    }
    else if(data_type == QVariant::String)
    {
        data_typeStr = QString("varchar");
        sqlstr = QString("alter table %1 change  `%2`  `%3` %4(255) ").arg(this->mTableName).arg(oldName).arg(newName)
                                                                            .arg(data_typeStr);
    }
    if(!query.exec(sqlstr))
    {
       QMessageBox::warning(nullptr,"error","修改失败！",QMessageBox::Yes);
    }
    else
    {
        //保存修改值
        if(model->isDirty())
        {
            int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
            if(isSave == QMessageBox::Yes)
            {
                model->submitAll();
                tableViewInit_page();
                model->select();
            }
        }
        //刷新表格
        model->setTable(this->mTableName);
        //重新查询
        tableViewInit_page();
        model->select();
        qDebug()<<"[database] 修改字段名成功 datatype:"<<data_typeStr<<QString(" length:%1 precision:%2 ").arg(data_length).arg(data_precision);
    }


}


void DatabaseWidget::on_displayRowCount_editingFinished()
{
    int temp = ui->displayRowCount->text().toInt();
    if(temp > 0)
    {
        this->mOnepageCount = temp;
    }
    else
    {
        QMessageBox::warning(nullptr,"error",QString("设置值应大于0"),QMessageBox::Yes);
    }
    this->mCurrentPage = 0;
    tableViewInit_page();
    model->select();
}

void DatabaseWidget::showContextMenu(const QPoint &point)
{
    //保存修改值
    if(model->isDirty())
    {
        int isSave = QMessageBox::warning(this,"warning","是否保存当前对数据库的修改",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
        if(isSave == QMessageBox::Yes)
        {
            model->submitAll();
            tableViewInit_page();
            model->select();
        }
    }
    QPoint globalPos = ui->tableView->mapToGlobal(point);
    int column = ui->tableView->horizontalHeader()->logicalIndexAt(point);
    QString field_name = model->headerData(column,Qt::Horizontal).toString();
    QVariant fieldType = model->record().field(column).type();
    int data_length = model->record().field(column).length();
    int data_precision = model->record().field(column).precision();
    QMenu myMenu;
    //插入一列
    QAction *insertColumnAction = new QAction(tr("插入一列"), this);
    connect(insertColumnAction, &QAction::triggered, [=]() {
        QSqlQuery query;
        bool isOK;
        QString name =QInputDialog::getText(this,tr("编辑标题名"),tr("输入新标题:")
                                               ,QLineEdit::Normal,model->headerData(column,Qt::Horizontal).toString(),&isOK);
        if(isOK)
        {
            if(!query.exec(QString("ALTER TABLE %1 ADD COLUMN `%2` varchar(255) AFTER `%3`").arg(this->mTableName).arg(name).arg(field_name)))
            {
                qDebug()<<"[database] 插入失败:"<<QString("ALTER TABLE %1 ADD COLUMN `%2` varchar(255) AFTER `%3`").arg(this->mTableName).arg(name).arg(field_name);
                QMessageBox::warning(nullptr,"error","插入失败！",QMessageBox::Yes);
                return;
            }
            //刷新表格
            model->setTable(this->mTableName);
            //重新查询
            tableViewInit_page();
            model->select();
            qDebug()<<"[database] 插入一列";
        }
        else
            QMessageBox::warning(nullptr,"error","输入字段名无效！",QMessageBox::Yes);
    });
    //删除一列
    QAction *deleteColumnAction = new QAction(tr("删除一列"), this);
    connect(deleteColumnAction, &QAction::triggered, [=]() {
        QSqlQuery query;
        if(!query.exec(QString("ALTER TABLE `%1` DROP COLUMN `%2`").arg(this->mTableName).arg(field_name)))
        {
            qDebug()<<"[database] 删除失败："<<QString("ALTER TABLE `%1` DROP COLUMN `%2`").arg(this->mTableName).arg(field_name);
            return;
        }
        //刷新表格
        model->setTable(this->mTableName);
        //重新查询
        tableViewInit_page();
        model->select();
        qDebug()<<"[database] 删除一列";
    });
    //移动一列
    QAction *moveColumnAction = new QAction(tr("移动一列"), this);
    connect(moveColumnAction, &QAction::triggered, [=]() {
        QSqlQuery query;
        bool isOK;
        QString dstFieldName =QInputDialog::getText(this,tr("移动列"),tr("插入至:")
                                               ,QLineEdit::Normal,model->headerData(column,Qt::Horizontal).toString(),&isOK);
        bool isFieldName = false;
        if(isOK)
        {
            QSqlRecord  record = model->record();
            for(int i = 0;i<record.count();++i)
            {
                QString field = record.fieldName(i);
                if(field == dstFieldName)
                {
                    isFieldName = true;
                    break;
                }
            }
        }
        if(!isFieldName)
        {
            QMessageBox::warning(nullptr,"error","输入目标列无效",QMessageBox::Yes);
        }
        else
        {
            QString text ;
            if(fieldType == QVariant::Double)
            {
                text = QString("double(%1,%2)").arg(data_length).arg(data_precision);
            }
            else if(fieldType == QVariant::String)
            {
                text = QString("varchar(255)");
            }
            else
            {
                QMessageBox::warning(nullptr,"提示","该列不可移动",QMessageBox::Yes);
                return;
            }
            QString sql1 = QString("alter table %1 add column tempCol %2").arg(this->mTableName).arg(text);
            //sql2有可能报错
            QString sql2 = QString("update %1 set tempCol = `%2` ,`%3` = NULL").arg(this->mTableName).arg(field_name).arg(field_name);
            QString sql3 = QString("alter table %1 drop column `%2`").arg(this->mTableName).arg(field_name);
            QString sql4 = QString("alter table %1 change tempCol `%2` %3 after `%4`").arg(this->mTableName).arg(field_name).arg(text).arg(dstFieldName);
            if(query.exec(sql1)&&query.exec(sql2)&&query.exec(sql3)&&query.exec(sql4))
            {
                //刷新表格
                model->setTable(this->mTableName);
                //重新查询
                tableViewInit_page();
                model->select();
                qDebug()<<"[database] 移动一列";
            }
            else
            {
                QMessageBox::warning(nullptr,"error","移动列失败，请注意是否增加了新列！",QMessageBox::Yes);
                qDebug()<<query.lastQuery();
            }
        }
    });
    myMenu.addAction(insertColumnAction);
    myMenu.addAction(deleteColumnAction);
    myMenu.addAction(moveColumnAction);
    //显示上下文菜单
    //对于一些特殊的列不能执行上下文菜单  比如时间列、产品编号列、操作员编号列
    if(field_name == QString("产品编号") || field_name == QString("测试时间") ||field_name == QString("操作员编号"))
    {
        return;
    }
    myMenu.exec(globalPos);
}



DatabaseWidget::~DatabaseWidget()
{
    delete ui;
}


