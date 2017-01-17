
#include <QApplication>
#include <QDebug>
#include <clocale>

#include "projectwindow.h"

int main(int argc, char *argv[])
{
    qSetMessagePattern("[%{time yyyy-MM-dd h:mm:ss.zzz}%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}][%{file}:%{line} %{function}] %{message}");
    char *lc;
    if (!(lc =setlocale (LC_ALL, NULL))) {
        qDebug() << "Error setting locale.";
    }
    qDebug() << "Using locale " << lc;

    QApplication a(argc, argv);
    ProjectWindow w;
    a.installEventFilter(&w); // Pass events from a to w
    w.show();

    return a.exec();
}
