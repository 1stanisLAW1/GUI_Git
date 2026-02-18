#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(900,600);
    w.setWindowTitle("Git_GUI");
    w.show();
    return a.exec();
}
