#include "mainwindow.h"
#include "ui_mainwindow.h"
void PaintWindow::paintEvent(QPaintEvent*event)
{
    setMinimumSize(paintTreeX,paintTreeY);
    QPainter painter(this);
    QPen pen; //画笔
    QBrush brush; //画刷
    QFont font;
    pen.setColor(QColor(Qt::black));
    brush.setColor(QColor(255, 0, 0, 125));
    font.setFamily("宋体");
    //设置文字大小为50像素
    font.setPixelSize(7);
    //设置文字为粗体
    font.setBold(false);             //封装的setWeight函数
    //设置文字为斜体
    font.setItalic(false);           //封装的setStyle函数
    painter.setPen(pen); //添加画笔
    painter.setBrush(brush); //添加画刷
    painter.setFont(font);
    int d=10;

    if(paintTree.size()!=0)
    {
        vector<int> idstack;
        idstack.push_back(0);
        while (!idstack.empty())
        {
            int nownode = idstack.front();
            idstack.erase(idstack.begin());
            for(auto i=0;i<paintTree[nownode].children.size();i++)
                idstack.push_back(paintTree[nownode].children[i]);
            if(nownode!=0)
            {
                painter.drawLine(QPoint(paintTree[paintTree[nownode].parent].x,paintTree[paintTree[nownode].parent].y),QPoint(paintTree[nownode].x,paintTree[nownode].y));
            }
            painter.drawEllipse(paintTree[nownode].x-d/2,paintTree[nownode].y-d/2,d,d);
            painter.drawText(paintTree[nownode].x+d/2,paintTree[nownode].y+d,QString::fromLocal8Bit(paintTree[nownode].name.data()));
        }
    }

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    editor=new QsciScintilla(this);
    //设置语法
    QsciLexerCPP *textLexer = new QsciLexerCPP;//创建一个词法分析器

    textLexer->setColor(QColor(Qt::gray),QsciLexerCPP::Default);
    textLexer->setColor(QColor(Qt::gray),QsciLexerCPP::Comment);
    textLexer->setColor(QColor(167,127,164),QsciLexerCPP::Number);
    textLexer->setColor(QColor(244,153,48),QsciLexerCPP::DoubleQuotedString);
    textLexer->setColor(QColor(244,153,48),QsciLexerCPP::SingleQuotedString);
    textLexer->setColor(QColor(0,145,198),QsciLexerCPP::Keyword);
    textLexer->setColor(QColor(139,142,40),QsciLexerCPP::Identifier);

    editor->setLexer(textLexer);//给QsciScintilla设置词法分析器

    //行号提示
    editor->setMarginType(0,QsciScintilla::NumberMargin);//设置编号为0的页边显示行号。
    editor->setMarginLineNumbers(0,true);//对该页边启用行号
    editor->setMarginWidth(0,50);//设置页边宽度

    //代码提示
    QsciAPIs *apis = new QsciAPIs(textLexer);
    apis->add(QString("int"));
    apis->add(QString("void"));
    apis->add(QString("if"));
    apis->add(QString("else"));
    apis->add(QString("while"));
    apis->add(QString("return"));
    apis->prepare();

    editor->setAutoCompletionSource(QsciScintilla::AcsAll);   //设置源，自动补全所有地方出现的
    editor->setAutoCompletionCaseSensitivity(true);   //设置自动补全大小写敏感
    editor->setAutoCompletionThreshold(1);    //设置每输入一个字符就会出现自动补全的提示
    editor->setFont(QFont("Courier New"));//设置显示字体
    editor->SendScintilla(QsciScintilla::SCI_SETCODEPAGE,QsciScintilla::SC_CP_UTF8);//设置编码为UTF-8

    auto pLayout = new QVBoxLayout(ui->ewidget);
    pLayout->addWidget(editor);
    pLayout->setContentsMargins(0,0,0,0);

    pw = new PaintWindow(ui->scrollAreaWidgetContents_rtree);
    auto paint=new QVBoxLayout(ui->scrollAreaWidgetContents_rtree);
    paint->addWidget(pw);
    pw->update();
//    ui->scrollAreaWidgetContents_rtree->setWidget(pw);
//    pw->update();

    connect(editor,SIGNAL(textChanged()),this,SLOT(on_editor_changed()));
    connect(ui->action_quit,SIGNAL(triggered()),this,SLOT(close()));

    flushTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent*event)
{
    if(!is_save)
    {
        QMessageBox::StandardButton result;//返回选择的按钮
        result=QMessageBox::question(this, "消息", "当前文件未保存，是否保存文件？", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,QMessageBox::Yes);
        if (result==QMessageBox::Yes)
        {
            if(!on_action_save_triggered())
                event->ignore();
        }
        else if(result==QMessageBox::No){}
        else
        {
            event->ignore();
        }
    }
}

void MainWindow::flushTitle()
{
    if(is_save)
    {
        this->setWindowTitle(QObject::tr((program_name + " - " + editfile_name).c_str()));
    }
    else
    {
        this->setWindowTitle(QObject::tr((program_name + "* - " + editfile_name).c_str()));
    }
}

void MainWindow::compile()
{
    QString str=editor->text();
    string fstring = string(str.toLocal8Bit());

    tm t;
    time_t now;
    time(&now);
    localtime_s(&t, &now);
    QString time = QString("%1:%2:%3 ").arg(t.tm_hour).arg(t.tm_min).arg(t.tm_sec);
    ui->textBrowser->append("＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋");
    ui->textBrowser->append("混元形译编译器");
    ui->textBrowser->append(time);
    ui->textBrowser->append("开发者:https://github.com/lingbai-kong");
    ui->textBrowser->append("【开始编译】");
    ui->textBrowser->update();
    try {
        ui->textBrowser->append("正在生成中间代码...");
        ui->textBrowser->update();
        syntaxAnalysis syntax;
        syntax.initializeLR1();
        syntax.getInput(fstring);
        syntax.analysis();
        ui->textBrowser->append("正在优化中间代码...");
        ui->textBrowser->update();
        optimizerAnalysis optimizer(syntax.L.nameTable, syntax.S.global_table, syntax.S.intermediate_code);
        double opt_rate = optimizer.analysis();
        ui->textBrowser->append("优化率:"+QString::number(opt_rate, 'f', 2)+"%");
        ui->textBrowser->append("正在生成目标代码...");
        ui->textBrowser->update();
        objectCodeGenerator MIPSgenerator(optimizer.intermediate_code, optimizer.block_group,stack_size);
        MIPSgenerator.geneObjectCode();

        ui->textBrowser->append("正在保存目标代码...");
        ui->textBrowser->update();
        string MIPSfile_name=editfile_name.substr(0,editfile_name.find_last_of("."));
        MIPSfile_name+=".s";
        fstream fout(MIPSfile_name, ios::out);
        for (auto i = 0; i < MIPSgenerator.object_code.size(); i++)
            fout << MIPSgenerator.object_code[i] << endl;
        fout.close();

        ui->textBrowser->append("目标代码以保存至"+QString::fromLocal8Bit(MIPSfile_name.data()));
        ui->textBrowser->append("正在生成结果信息...");
        ui->textBrowser->update();
        ui->textBrowser->append("【编译成功】");
        ui->textBrowser->append("＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋\n");


        ui->tableWidget_sym_stream->clearContents();
        ui->tableWidget_sym_stream->setRowCount(syntax.L.history.size());
        for (auto i = 0; i < syntax.L.history.size();i++)
        {
            ui->tableWidget_sym_stream->setItem(i,0,new QTableWidgetItem(syntax.L.history[i].first.data()));
            ui->tableWidget_sym_stream->setItem(i,1,new QTableWidgetItem(to_string(syntax.L.history[i].second).data()));
        }
        ui->tableWidget_sym_stream->resizeColumnsToContents();

        ui->tableWidget_name_table->clearContents();
        ui->tableWidget_name_table->setRowCount(syntax.L.nameTable.size());
        int name_table_count = 0;
        for (map<int, string>::iterator iter = syntax.L.nameTable.begin(); iter != syntax.L.nameTable.end(); iter++)
        {
            ui->tableWidget_name_table->setItem(name_table_count,0,new QTableWidgetItem(to_string(iter->first).data()));
            ui->tableWidget_name_table->setItem(name_table_count,1,new QTableWidgetItem(iter->second.data()));
            name_table_count++;
        }
        ui->tableWidget_name_table->resizeColumnsToContents();

        ui->tableWidget_LR1table->clearContents();
        ui->tableWidget_LR1table->setRowCount(syntax.G.ACTION.size());
        ui->tableWidget_LR1table->setColumnCount(syntax.G.VN.size()+syntax.G.VT.size());
        QStringList headers;
        for (auto i=0;i<syntax.G.VT.size();i++)
            headers << QString::fromLocal8Bit(syntax.G.VT[i].data());
        for (auto i=0;i<syntax.G.VN.size();i++)
            headers << QString::fromLocal8Bit(syntax.G.VN[i].data());
        ui->tableWidget_LR1table->setHorizontalHeaderLabels(headers);
        for (auto i=0;i<syntax.G.ACTION.size();i++)
        {
            for (auto j=0;j<syntax.G.ACTION[i].size();j++)
            {
                switch (syntax.G.ACTION[i][j].status)
                {
                case ACTION_ACC:ui->tableWidget_LR1table->setItem(i,j,new QTableWidgetItem("ACC")); break;
                case ACTION_STATE:ui->tableWidget_LR1table->setItem(i,j,new QTableWidgetItem(("s"+to_string(syntax.G.ACTION[i][j].nextState)).data())); break;
                case ACTION_REDUCTION:
                {
                    string context;
                    context="r:"+syntax.G.ACTION[i][j].p.first+"->";
                    for (int k = 0; k < syntax.G.ACTION[i][j].p.second.size(); k++)
                    {
                        context+=syntax.G.ACTION[i][j].p.second[k]+" ";
                    }
                    ui->tableWidget_LR1table->setItem(i,j,new QTableWidgetItem(context.data()));
                    break;
                }
                case ACTION_ERROR:break;
                default:break;
                }
            }
            for (auto j=0;j<syntax.G.GOTO[i].size();j++)
            {
                if(syntax.G.GOTO[i][j]!=-1)
                    ui->tableWidget_LR1table->setItem(i,j+syntax.G.ACTION[i].size(),new QTableWidgetItem(to_string(syntax.G.GOTO[i][j]).data()));
            }
        }
        //ui->tableWidget_LR1table->resizeColumnsToContents();

        ui->tableWidget_anls_steps->clearContents();
        ui->tableWidget_anls_steps->setRowCount(syntax.history.size());
        for (auto i=0;i<syntax.history.size();i++)
        {
            string context1;
            if(syntax.history[i][0].size()==0) context1="";
            else if(syntax.history[i][0].size()==1) context1=syntax.history[i][0][0];
            else if(syntax.history[i][0].size()==2) context1=syntax.history[i][0][0]+" "+syntax.history[i][0][1];
            else context1="..."+syntax.history[i][0][syntax.history[i][0].size()-2]+" "+syntax.history[i][0][syntax.history[i][0].size()-1];
            ui->tableWidget_anls_steps->setItem(i,0,new QTableWidgetItem(context1.data()));

            string context2;
            if(syntax.history[i][1].size()==0) context2="";
            else if(syntax.history[i][1].size()==1) context2=syntax.history[i][1][0];
            else if(syntax.history[i][1].size()==2) context2=syntax.history[i][1][0]+" "+syntax.history[i][1][1];
            else context2="..."+syntax.history[i][1][syntax.history[i][1].size()-2]+" "+syntax.history[i][1][syntax.history[i][1].size()-1];
            ui->tableWidget_anls_steps->setItem(i,1,new QTableWidgetItem(context2.data()));
            if(syntax.history[i][2][0]=="#")
                ui->tableWidget_anls_steps->setItem(i,2,new QTableWidgetItem((syntax.history[i][2][0]).data()));
            else
                ui->tableWidget_anls_steps->setItem(i,2,new QTableWidgetItem((syntax.history[i][2][0]+"...").data()));
        }
        ui->tableWidget_anls_steps->resizeColumnsToContents();

        ui->textBrowser_objcode->clear();
        for (auto i = 0; i < MIPSgenerator.object_code.size(); i++)
        {
            ui->textBrowser_objcode->append(QString::fromLocal8Bit(MIPSgenerator.object_code[i].data()));
        }

        calCoordinate(syntax.reductionTreeRoot,syntax.maxTreeLevel,syntax.leafNum);
        pw->update();

        while(ui->toolBox->count()!=0)
            ui->toolBox->removeItem(0);
        int table_count=1;

        for (symbolTable* tp = syntax.S.last_table;tp != NULL;tp = tp->previous)
        {
            if(tp->table.size()==0)
                continue;
            QTableWidget* newtable=new QTableWidget;
            QStringList headers;
            headers<<"名字"<<"性质"<<"类型"<<"地址"<<"维度";
            newtable->setHorizontalHeaderLabels(headers);
            newtable->setRowCount(tp->table.size());
            newtable->setColumnCount(5);
            for (auto i = 0; i < tp->table.size(); i++)
            {
                if(syntax.L.nameTable[tp->table[i].id]=="")
                    newtable->setItem(i,0,new QTableWidgetItem("临时变量"));
                else
                    newtable->setItem(i,0,new QTableWidgetItem(syntax.L.nameTable[tp->table[i].id].data()));
                if (tp->table[i].k == VAR)
                    newtable->setItem(i,1,new QTableWidgetItem("变量"));
                else if (tp->table[i].k == ARRAY)
                    newtable->setItem(i,1,new QTableWidgetItem("数组"));
                else if (tp->table[i].k == FUNC)
                    newtable->setItem(i,1,new QTableWidgetItem("函数"));
                if (tp->table[i].t == INT)
                    newtable->setItem(i,2,new QTableWidgetItem("INT"));
                else if (tp->table[i].t == VOID)
                    newtable->setItem(i,2,new QTableWidgetItem("VOID"));
                else
                    newtable->setItem(i,2,new QTableWidgetItem("NOTYPE"));
                newtable->setItem(i,3,new QTableWidgetItem(to_string(tp->table[i].offset).data()));
                if(tp->table[i].dimension.size()!=0)
                {
                    string dim="(";
                    for (auto j = 0; j < tp->table[i].dimension.size(); j++)
                        dim+=to_string(tp->table[i].dimension[j])+(j==tp->table[i].dimension.size()-1? ")":",");
                    newtable->setItem(i,4,new QTableWidgetItem(dim.data()));
                }
            }
            newtable->resizeColumnsToContents();
            string title;
            if(tp==syntax.S.global_table)
                title=string("顶层全局表")+string(" ")+to_string(tp->width)+string("字节");
            else
                title=string("表:")+to_string(table_count)+string(" ")+to_string(tp->width)+string("字节");
            QWidget *page = new QWidget;
            auto pLayout = new QGridLayout(page);
            pLayout->addWidget(newtable);
            //pLayout->setContentsMargins(0,0,0,0);
            page->setLayout(pLayout);
            //page->setMinimumHeight(1000);
            ui->toolBox->addItem(page,tr(title.data()));
            table_count++;
        }
        ui->toolBox->setMinimumHeight(30*table_count+240);

        ui->tableWidget_imcode->clearContents();
        ui->tableWidget_imcode->setRowCount(syntax.S.intermediate_code.size());
        for(auto i=0;i<syntax.S.intermediate_code.size();i++)
        {
            ui->tableWidget_imcode->setItem(i,0,new QTableWidgetItem(syntax.S.intermediate_code[i].op.data()));
            ui->tableWidget_imcode->setItem(i,1,new QTableWidgetItem(syntax.S.intermediate_code[i].arg1.data()));
            ui->tableWidget_imcode->setItem(i,2,new QTableWidgetItem(syntax.S.intermediate_code[i].arg2.data()));
            ui->tableWidget_imcode->setItem(i,3,new QTableWidgetItem(syntax.S.intermediate_code[i].result.data()));
        }
        ui->tableWidget_imcode->resizeColumnsToContents();

        while(ui->toolBox_block->count()!=0)
            ui->toolBox_block->removeItem(0);
        int block_count=1;
        int height=0;
        for(auto i=0;i<optimizer.block_group.size();i++)
        {
            QTableWidget* newtable=new QTableWidget;
            QStringList headers;
            headers<<"op"<<"arg1"<<"arg2"<<"result";
            newtable->setHorizontalHeaderLabels(headers);
            newtable->setRowCount(optimizer.block_group[i].end-optimizer.block_group[i].begin);
            newtable->setColumnCount(4);
            for(auto j=optimizer.block_group[i].begin;j<optimizer.block_group[i].end;j++)
            {
                newtable->setItem(j-optimizer.block_group[i].begin,0,new QTableWidgetItem(optimizer.intermediate_code[j].op.data()));
                newtable->setItem(j-optimizer.block_group[i].begin,1,new QTableWidgetItem(optimizer.intermediate_code[j].arg1.data()));
                newtable->setItem(j-optimizer.block_group[i].begin,2,new QTableWidgetItem(optimizer.intermediate_code[j].arg2.data()));
                newtable->setItem(j-optimizer.block_group[i].begin,3,new QTableWidgetItem(optimizer.intermediate_code[j].result.data()));
            }
            newtable->resizeColumnsToContents();
            string title=string("基本块:")+to_string(i)+string(" ")+string("(")+to_string(optimizer.block_group[i].begin)+string("-")+to_string(optimizer.block_group[i].end)+string(")");
            QWidget *page = new QWidget;
            auto pLayout = new QGridLayout(page);
            pLayout->addWidget(newtable);
            page->setLayout(pLayout);
            if(newtable->height()>height)
                height=newtable->height();
            ui->toolBox_block->addItem(page,tr(title.data()));
            block_count++;
        }
        ui->toolBox_block->setMinimumHeight(30*block_count+240);

        ui->tableWidget_optimcode->clearContents();
        ui->tableWidget_optimcode->setRowCount(optimizer.intermediate_code.size());
        for(auto i=0;i<optimizer.intermediate_code.size();i++)
        {
            ui->tableWidget_optimcode->setItem(i,0,new QTableWidgetItem(optimizer.intermediate_code[i].op.data()));
            ui->tableWidget_optimcode->setItem(i,1,new QTableWidgetItem(optimizer.intermediate_code[i].arg1.data()));
            ui->tableWidget_optimcode->setItem(i,2,new QTableWidgetItem(optimizer.intermediate_code[i].arg2.data()));
            ui->tableWidget_optimcode->setItem(i,3,new QTableWidgetItem(optimizer.intermediate_code[i].result.data()));
        }
        ui->tableWidget_imcode->resizeColumnsToContents();

        ui->tableWidget_objmsg->clearContents();
        ui->tableWidget_objmsg->setRowCount(MIPSgenerator.messageTableHistory.size());
        for (auto tno = 0; tno < MIPSgenerator.messageTableHistory.size(); tno++)
        {
            messageTableItem message_table = MIPSgenerator.messageTableHistory[tno];
            ui->tableWidget_objmsg->setItem(tno,0,new QTableWidgetItem(message_table.TAS.op.data()));
            ui->tableWidget_objmsg->setItem(tno,1,new QTableWidgetItem(message_table.TAS.arg1.data()));
            ui->tableWidget_objmsg->setItem(tno,2,new QTableWidgetItem(message_table.TAS.arg2.data()));
            ui->tableWidget_objmsg->setItem(tno,3,new QTableWidgetItem(message_table.TAS.result.data()));

            string left_value="";
            if(message_table.result_tag.second)
            {
                if(message_table.result_tag.first==INT_MAX)
                    left_value=string("(^,y)");
                else
                    left_value=string("("+to_string(message_table.result_tag.first)+",y)");
            }
            else
                left_value=string("(^,^)");
            ui->tableWidget_objmsg->setItem(tno,4,new QTableWidgetItem(left_value.data()));

            string left_arg="";
            if(message_table.arg1_tag.second)
            {
                if(message_table.arg1_tag.first==INT_MAX)
                    left_arg=string("(^,y)");
                else
                    left_arg=string("("+to_string(message_table.arg1_tag.first)+",y)");
            }
            else
                left_arg=string("(^,^)");
            ui->tableWidget_objmsg->setItem(tno,5,new QTableWidgetItem(left_arg.data()));

            string right_arg="";
            if(message_table.arg2_tag.second)
            {
                if(message_table.arg2_tag.first==INT_MAX)
                    right_arg=string("(^,y)");
                else
                    right_arg=string("("+to_string(message_table.arg2_tag.first)+",y)");
            }
            else
                right_arg=string("(^,^)");
            ui->tableWidget_objmsg->setItem(tno,6,new QTableWidgetItem(right_arg.data()));
        }
        ui->tableWidget_objmsg->resizeColumnsToContents();

        ui->tableWidget_objreg->clearContents();
        ui->tableWidget_objreg->setRowCount(MIPSgenerator.analysisHistory.size());
        for (auto i=0;i<MIPSgenerator.analysisHistory.size();i++)
        {
            ui->tableWidget_objreg->setItem(i,0,new QTableWidgetItem(MIPSgenerator.analysisHistory[i].TAS.op.data()));
            ui->tableWidget_objreg->setItem(i,1,new QTableWidgetItem(MIPSgenerator.analysisHistory[i].TAS.arg1.data()));
            ui->tableWidget_objreg->setItem(i,2,new QTableWidgetItem(MIPSgenerator.analysisHistory[i].TAS.arg2.data()));
            ui->tableWidget_objreg->setItem(i,3,new QTableWidgetItem(MIPSgenerator.analysisHistory[i].TAS.result.data()));
            string object_codes="";
            for (auto j = 0; j < MIPSgenerator.analysisHistory[i].object_codes.size(); j++)
            {
                object_codes+=MIPSgenerator.analysisHistory[i].object_codes[j]+(j==MIPSgenerator.analysisHistory[i].object_codes.size()-1? "":"\n");
            }
            ui->tableWidget_objreg->setItem(i,4,new QTableWidgetItem(object_codes.data()));
            string RVALUE="";
            for (map<string, vector<pair<string, int>>>::iterator iter = MIPSgenerator.analysisHistory[i].RVALUE.begin(); iter != MIPSgenerator.analysisHistory[i].RVALUE.end(); iter++)
            {
                RVALUE+=iter->first+": ";
                for (auto k = 0; k < iter->second.size(); k++)
                {
                    RVALUE+=iter->second[k].first+" ";
                }
                map<string, vector<pair<string, int>>>::iterator olditer=iter;
                iter++;
                RVALUE+=(iter==MIPSgenerator.analysisHistory[i].RVALUE.end()? "":"\n");
                iter=olditer;
            }
            ui->tableWidget_objreg->setItem(i,5,new QTableWidgetItem(RVALUE.data()));
            string AVALUE="";
            for (map<string, vector<string>>::iterator iter = MIPSgenerator.analysisHistory[i].AVALUE.begin(); iter != MIPSgenerator.analysisHistory[i].AVALUE.end(); iter++)
            {
                AVALUE+=iter->first+": ";
                for (auto k = 0; k < iter->second.size(); k++)
                {
                    AVALUE+=iter->second[k]+" ";
                }
                map<string, vector<string>>::iterator olditer=iter;
                iter++;
                AVALUE+=(iter==MIPSgenerator.analysisHistory[i].AVALUE.end()? "":"\n");
                iter=olditer;
            }
            ui->tableWidget_objreg->setItem(i,6,new QTableWidgetItem(AVALUE.data()));
        }
        ui->tableWidget_objreg->resizeRowsToContents();
        ui->tableWidget_objreg->resizeColumnsToContents();
    } catch (string expmsg) {
        QString Qexpmsg = QString::fromLocal8Bit(expmsg.data());
        ui->textBrowser->append(Qexpmsg);
        ui->textBrowser->append("【编译失败】");
        ui->textBrowser->append("＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋＋\n");
    }

    //syntax.S.showTables(syntax.L.symbolTable);
    //syntax.S.showIntermediateCode();

    //optimizer.showIntermediateCode();
    //optimizer.showBlockGroup();
    //optimizer.showDAG();

    //MIPSgenerator.showObjectCode();
    //MIPSgenerator.showMessageTableHistory();
    //MIPSgenerator.showAnalysisHistory();

    //return MIPSgenerator.object_code;
}

void MainWindow::calCoordinate(treeNode* root,int deep,int leaf_num)
{
    int xMargin=100;//绘图区左右边框宽度
    int yMargin=100;//绘图区上下边框宽度
    int xUnit=40;//每个叶子结点横坐标距离
    int yUnit=40;//每层结点纵坐标距离
    int xMax=32767;//绘图区最大宽度
    int yMax=32767;//绘图区最大高度
    pw->paintTreeX=xUnit*leaf_num+2*xMargin;//计算绘图区宽度
    pw->paintTreeY=yUnit*deep+2*yMargin;//计算绘图区高度
    int xCount=0;//#已生成坐标的叶子结点数目
    treeNode* curNode=root;
    if(pw->paintTreeX>xMax)//当绘图区宽度超限制时,重新计算参数
    {
        xUnit=(xMax-2*xMargin)/leaf_num;
        pw->paintTreeX=xMax;
    }
    if(pw->paintTreeY>yMax)//当绘图区高度超限制时,重新计算参数
    {
        yUnit=(yMax-2*yMargin)/deep;
        pw->paintTreeY=yMax;
    }
    while(true)//#主循环
    {
        bool down=false;//进入下一层标志,初始为假
        treeNode* nextNode;
        for(auto i=0;i<curNode->children.size();i++)
        {
            nextNode=curNode->children[i];
            if(nextNode->x==-1&&nextNode->y==-1)//找到一个未生成坐标的子结点
            {
                if(nextNode->children.size()>0)//这个子结点还有后继
                {
                    down=true;//进入下一层标志为正
                    break;//跳出该循环
                }
                else//这个子结点没有后继,就是叶子结点
                {
                    //计算这个结点的坐标
                    nextNode->x=xMargin+xCount*xUnit;
                    nextNode->y=yMargin+nextNode->level*yUnit;
                    xCount++;//已生成坐标的叶子结点数目增1
                }
            }
        }
        if(down)//如果进入下一层标志为真
            curNode=nextNode;//以这个未生成坐标的子结点为当前结点进入下一次主循环
        else//进入下一层标志为假,说明当前结点的子结点均有坐标
        {
            int childrenSumX=0;//子结点横坐标和
            int childrenNum=curNode->children.size();//子结点数目
            for(auto i=0;i<childrenNum;i++)//计算子结点横坐标之和
                childrenSumX+=curNode->children[i]->x;
            //计算当前结点的坐标
            curNode->x=childrenSumX/childrenNum;
            curNode->y=yMargin+curNode->level*yUnit;
            if(curNode==root)//如果当前结点是根结点,说明所有结点坐标均已计算完成
                break;//退出主循环
            curNode=curNode->parent;//否则回溯到当前结点的前驱进入下一次主循环
        }
    }

    pw->paintTree.clear();
    paintTreeNode proot;
    proot.x=root->x;
    proot.y=root->y;
    proot.name=root->content.first;
    proot.parent=-1;
    vector<treeNode*> treeQueue;
    treeQueue.push_back(root);
    vector<paintTreeNode> ptreeQueue;
    ptreeQueue.push_back(proot);
    treeNode* nowNode=root;
    paintTreeNode pnowNode=proot;
    while(!treeQueue.empty())
    {
        nowNode=treeQueue.front();
        treeQueue.erase(treeQueue.begin());
        pnowNode=ptreeQueue.front();
        ptreeQueue.erase(ptreeQueue.begin());


        pnowNode.no=pw->paintTree.size();
        if(pnowNode.parent!=-1)
            pw->paintTree[pnowNode.parent].children.push_back(pnowNode.no);
        pw->paintTree.push_back(pnowNode);

        for(auto i=0;i<nowNode->children.size();i++)
        {
            treeNode* newNode=nowNode->children[i];
            paintTreeNode pnewNode;
            pnewNode.x=newNode->x;
            pnewNode.y=newNode->y;
            pnewNode.name=newNode->content.first;
            pnewNode.parent=pnowNode.no;

            treeQueue.push_back(newNode);
            ptreeQueue.push_back(pnewNode);
        }

    }
}

void MainWindow::on_editor_changed()
{
    is_save = false;
    flushTitle();
}

void MainWindow::on_action_open_triggered()
{
    if(!is_save)
    {
        QMessageBox::StandardButton result;//返回选择的按钮
        result=QMessageBox::question(this, "消息", "当前文件未保存，是否保存文件？", QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,QMessageBox::Yes);
        if (result==QMessageBox::Yes)
        {
            if(!on_action_save_triggered())
                return;
        }
        else if(result==QMessageBox::No){}
        else
        {
            return;
        }
    }

    QString file_name = QFileDialog::getOpenFileName(this, tr("打开文件"), "./", tr("源文件(*.c *.cpp);;txt文件(*.txt)"));
    if (file_name.isEmpty())
    {
        return;
    }
    editfile_name = string(file_name.toLocal8Bit());

    fstream f(editfile_name, ios::in);
    stringstream buffer;
    string fstring;
    vector<string> objcode;
    buffer << f.rdbuf();
    fstring = buffer.str();
    f.close();
    QString qstring = QString::fromLocal8Bit(fstring.data());
    editor->setText(qstring);
    is_save = true;
    flushTitle();
}

void MainWindow::on_action_new_triggered()
{
    if(!is_save)
    {
        QMessageBox::StandardButton result;//返回选择的按钮
        result=QMessageBox::question(this, "消息", "当前文件未保存，是否保存文件？", QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,QMessageBox::Yes);
        if (result==QMessageBox::Yes)
        {
            if(!on_action_save_triggered())
                return;
        }
        else if(result==QMessageBox::No){}
        else
        {
            return;
        }
    }
    editfile_name=default_file_name;
    editor->clear();
    is_save = true;
    flushTitle();
}

bool MainWindow::on_action_save_triggered()
{
    QString str=editor->text();
    string fstring = string(str.toLocal8Bit());
    if(editfile_name==default_file_name)
    {
        if(!on_action_saveas_triggered())
            return false;
    }
    else
    {
        fstream fout(editfile_name,ios::out);
        fout<<fstring;
        fout.close();
    }
    is_save = true;
    flushTitle();
    return true;
}

bool MainWindow::on_action_saveas_triggered()
{
    QString str=editor->text();
    string fstring = string(str.toLocal8Bit());
    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="保存文件"; //对话框标题
    QString filter="文本文件(*.txt);;h文件(*.h);;C++文件(.cpp);;所有文件(*.*)"; //文件过滤器
    QString newFileName=QFileDialog::getSaveFileName(this,"保存文件","./","类C语言文件(.c)");
    if (newFileName.isEmpty())
    {
        return false;
    }
    fstream fout(string(newFileName.toLocal8Bit())+".c",ios::out);
    fout<<fstring;
    fout.close();
    editfile_name=string(newFileName.toLocal8Bit());
    is_save = true;
    flushTitle();
    return true;
}

void MainWindow::on_action_undo_triggered()
{
    editor->undo();
}

void MainWindow::on_action_redo_triggered()
{
    editor->redo();
}

void MainWindow::on_action_copy_triggered()
{
    editor->copy();
}

void MainWindow::on_action_cut_triggered()
{
    editor->cut();
}

void MainWindow::on_action_paste_triggered()
{
    editor->paste();
}

void MainWindow::on_action_compile_triggered()
{
    if(on_action_save_triggered())
        compile();
}

void MainWindow::on_pushButton_clearLog_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_action_triggered()
{
    QString dlgTitle="设置";
    QString txtLabel="设置堆栈区大小(KB)";
    int defaultValue=stack_size;
    int minValue=1, maxValue=1024,stepValue=1; //范围，步长
    bool ok=false;
    int inputValue = QInputDialog::getInt(this, dlgTitle,txtLabel,
                               defaultValue, minValue,maxValue,stepValue,&ok);
    if (ok) //是否确认输入
    {
        stack_size=inputValue;
    }
}
