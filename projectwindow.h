#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>
#include <QFile>

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
    void updateMenus();
    void setEditorText(QString code);
    void saveProject();

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
    QFile m_codeFile;
};

#endif // PROJECTWINDOW_H
