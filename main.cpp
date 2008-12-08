/*
 * @Name : ftm - Franklin's Task Manager
 * @Author: Alexey 'l1feh4ck3r' Antonov
 */

#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
