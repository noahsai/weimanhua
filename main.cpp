#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Wei漫画");
    w.setWindowIcon(QIcon(":/icon/weimanhua.png"));
    w.show();

    return a.exec();
}
