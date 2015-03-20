
#include <QApplication>
#include <QDebug>
#include <clocale>

#include "projectwindow.h"

int main(int argc, char *argv[])
{
    char *lc;
    if (!(lc =setlocale (LC_ALL, NULL))) {
        qDebug() << "Error setting locale.";
    }
    qDebug() << "Using locale " << lc;

    QApplication a(argc, argv);

    ProjectWindow w;
    w.show();
//    MainWindow w;
//    w.show();

    return a.exec();
}
