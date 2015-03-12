#include "projectwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ProjectWindow w;
    w.show();
//    MainWindow w;
//    w.show();

    return a.exec();
}
