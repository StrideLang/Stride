#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include <QMenu>
#include <QWebEngineView>

#include "languagehighlighter.h"
#include "builder.h"
#include "searchwidget.h"
#include "codemodel.hpp"

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
    void openGeneratedDir();
    void updateCodeAnalysis();
    void newFile();
    void markModified();

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void build();
    void flash();
    void run(bool pressed);
    void stop();
    void programStopped();
    void tabChanged(int index);
    bool maybeSave();
    void showDocumentation();
    void followSymbol();

    // Editor
    void commentSection();
    void uncomment();
    void showHelperMenu(QPoint where);
    void insertText(QString text = "");
    void find(QString query = "");
    void findNext();
    void findPrevious();

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
    QWebEngineView *docBrowser;
    QScopedPointer<SearchWidget> m_searchWidget;

    LanguageHighlighter *m_highlighter;

//    QString m_platformsRootDir;
    QMap<QString, QVariant> m_options;
    QMap<QString, QVariant> m_environment;
    QTimer m_codeModelTimer;
    CodeModel m_codeModel;
    QFont m_font;
    Builder *m_builder;
    QMenu m_helperMenu;
    bool m_startingUp;
};

#endif // PROJECTWINDOW_H
