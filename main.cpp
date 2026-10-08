#include <QApplication>
#include "mainwindow.h"

/**
 * 程序入口：
 * 创建主窗口并进入 Qt 事件循环。
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
