#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>

#include "baseproject.h"
#include "languagehighlighter.h"

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProjectWindow(QWidget *parent = 0, QString baseProjectDir = QString());
    ~ProjectWindow();

public slots:
    void updateMenus();
    void setEditorText(QString code);
    void saveFile();
    bool saveFileAs();
    void loadFile();
    void loadFile(QString fileName);
    void openOptionsDialog();
    void updateCodeAnalysis();
    void newFile();

private slots:
    void build();
    void flash();
    void run(bool pressed);
    void tabChanged(int index);

    void programStopped();

    void printConsoleText(QString text);
    void printConsoleError(QString text);

private:
//    void createMenus();
    void connectActions();

    void readSettings();
    void writeSettings();

    void updateEditorFont();

    QString makeProjectForCurrent();

    Ui::ProjectWindow *ui;
    QWidget *m_layoutContainer;
//    QToolBar *m_toolBar;

//    BaseProject *m_project;
//    QFile m_codeFile;
    LanguageHighlighter *m_highlighter;

    QString m_platformsRootDir;
    QMap<QString, QVariant> m_options;
    QTimer m_timer;
    QFont m_font;
};

#endif // PROJECTWINDOW_H
