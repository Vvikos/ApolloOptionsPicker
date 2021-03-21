#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qDebug("Starting Apollo Options Picker...");
    w.show();

    return a.exec();
}
