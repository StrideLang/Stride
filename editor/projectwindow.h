#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>

#include "languagehighlighter.h"
#include "pythonproject.h"

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ProjectWindow(QWidget *parent = 0);
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
    void stop();
    void tabChanged(int index);

    void programStopped();

    void printConsoleText(QString text);
    void printConsoleError(QString text);

private:
    void connectActions();

    void readSettings();
    void writeSettings();

    void updateEditorFont();

    QString makeProjectForCurrent();

    Ui::ProjectWindow *ui;
    QWidget *m_layoutContainer;

    LanguageHighlighter *m_highlighter;

    QString m_platformsRootDir;
    QMap<QString, QVariant> m_options;
    QTimer m_timer;
    QFont m_font;
    PythonProject *m_project;
};

#endif // PROJECTWINDOW_H
