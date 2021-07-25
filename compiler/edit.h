#ifndef EDIT_H
#define EDIT_H
//QsciScintilla作为QWidget的控件，需要添加该控件的头文件
#include <Qsci/qsciscintilla.h>
//以C++语法作为例子，该语法分析器的头文件
#include <Qsci/qscilexercpp.h>
//设置代码提示功能，依靠QsciAPIs类实现
#include <Qsci/qsciapis.h>

#include <QWidget>
#include <QGridLayout>
class editWidget : public QWidget
{
public:
    editWidget(QWidget *parent = nullptr);
};

#endif // EDIT_H
