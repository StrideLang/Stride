#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include <QMenu>

#include "languagehighlighter.h"
#include "builder.h"

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
    bool saveFile(int index = -1);
    bool saveFileAs(int index = -1);
    void closeTab(int index = -1);
    void loadFile();
    void loadFile(QString fileName);
    void openOptionsDialog();
    void updateCodeAnalysis();
    void newFile();
    void markModified();

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void build();
    void flash();
    void run(bool pressed);
    void stop();
    void programStopped();
    void tabChanged(int index);
    bool maybeSave();
    void showDocumentation();

    // Editor
    void commentSection();
    void uncomment();
    void showHelperMenu(QPoint where);
    void insertText(QString text = "");

    void printConsoleText(QString text);
    void printConsoleError(QString text);

private:
    void connectActions();
    void connectShortcuts();

    void readSettings();
    void writeSettings();

    void updateEditorFont();

    QString makeProjectForCurrent();

    Ui::ProjectWindow *ui;
    QWidget *m_layoutContainer;

    LanguageHighlighter *m_highlighter;

    QString m_platformsRootDir;
    QMap<QString, QVariant> m_options;
    QTimer m_codeModelTimer;
    QFont m_font;
    Builder *m_builder;
    QMutex m_validTreeLock;
    QMenu m_helperMenu;
    AST *m_lastValidTree;
    QList<AST *> m_platformObjects;
    bool m_startingUp;
};

#endif // PROJECTWINDOW_H
