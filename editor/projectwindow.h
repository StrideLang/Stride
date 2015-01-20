#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QWidget>
#include <QToolBar>

#include "baseproject.h"
#include "languagehighlighter.h"

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QWidget
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
    void createMenus();
    void connectActions();

    Ui::ProjectWindow *ui;
    QToolBar *m_toolBar;

    BaseProject *m_project;
    QFile m_codeFile;
    LanguageHighlighter *m_highlighter;
};

#endif // PROJECTWINDOW_H
