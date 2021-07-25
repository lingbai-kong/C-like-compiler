#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ctime>
#include <stdio.h>
#include "object_code.h"

#include <QMainWindow>
#include <QWidget>
#include <QGridLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QPainter>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPoint>
#include <global.h>

//QsciScintilla作为QWidget的控件，需要添加该控件的头文件
#include <Qsci/qsciscintilla.h>
//以C++语法作为例子，该语法分析器的头文件
#include <Qsci/qscilexercpp.h>
//设置代码提示功能，依靠QsciAPIs类实现
#include <Qsci/qsciapis.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct paintTreeNode
{
    int no;
    int parent;
    vector<int> children;
    string name;
    int x;
    int y;
};

class PaintWindow : public QWidget
{
    Q_OBJECT
public:
    vector<paintTreeNode>paintTree;
    int paintTreeX=0;
    int paintTreeY=0;
    PaintWindow(QWidget*parent=nullptr):QWidget(parent){};
protected:
    //添加重绘事件处理函数的声明
    void paintEvent(QPaintEvent*event);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_editor_changed();

    void on_action_open_triggered();

    void on_action_new_triggered();

    bool on_action_save_triggered();

    bool on_action_saveas_triggered();

    void on_action_undo_triggered();

    void on_action_redo_triggered();

    void on_action_copy_triggered();

    void on_action_cut_triggered();

    void on_action_paste_triggered();

    void on_action_compile_triggered();

    void on_pushButton_clearLog_clicked();

    void on_action_triggered();

protected:
    //这是一个虚函数，继承自QEvent.只要重写了这个虚函数，当你按下窗口右上角的"×"时，就会调用你所重写的此函数.
    void closeEvent(QCloseEvent*event);

private:
    Ui::MainWindow *ui;
    QsciScintilla *editor;
    PaintWindow *pw;
    const string program_name = "混元形译";
    const string default_file_name = "未命名";
    string editfile_name = "未命名";
    bool is_save = true;
    int stack_size=128;
    void flushTitle();
    void compile();
    void calCoordinate(treeNode* root,int deep,int leaf_num);
};
#endif // MAINWINDOW_H
