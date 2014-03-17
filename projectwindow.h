#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>

#include "baseproject.h"

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProjectWindow(QWidget *parent = 0, QString projectDir = QString());
    ~ProjectWindow();

public slots:
    void setView(bool simple);
    void updateMenus();

private slots:
    void build();
    void flash();
    void run(bool pressed);

    void programStopped();

    void setTargetFromMenu();

    void printConsoleText(QString text);
    void printConsoleError(QString text);

private:
    void connectActions();

    Ui::ProjectWindow *ui;
    QWidget *m_layoutContainer;
    BaseProject *m_project;
};

#endif // PROJECTWINDOW_H
