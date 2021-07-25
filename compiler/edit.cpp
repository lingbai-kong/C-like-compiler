#include "edit.h"
editWidget::editWidget(QWidget *parent)
    : QWidget(parent)
{

    QsciScintilla *editor=new QsciScintilla(this);
    //设置语法
    QsciLexerCPP *textLexer = new QsciLexerCPP;//创建一个词法分析器
    editor->setLexer(textLexer);//给QsciScintilla设置词法分析器

    //行号提示
    editor->setMarginType(0,QsciScintilla::NumberMargin);//设置编号为0的页边显示行号。
    editor->setMarginLineNumbers(0,true);//对该页边启用行号
    editor->setMarginWidth(0,40);//设置页边宽度

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

    auto pLayout = new QVBoxLayout(this);
    pLayout->addWidget(editor);
    pLayout->setContentsMargins(0,0,0,0);
}
