#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QProcess>
#include <QDebug>
#include <QActionGroup>
#include <QMessageBox>

#include "codeeditor.h"
#include "simpleproject.h"

ProjectWindow::ProjectWindow(QWidget *parent, QString projectDir) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    m_codeFile(projectDir + "/code/code.st")
{
    ui->setupUi(this);

    m_project = new SimpleProject(projectDir);
    m_project->setBoardId("0ontZocni8POZ");

    connectActions();

    QString name = projectDir.mid(projectDir.lastIndexOf("/") + 1);
    setWindowTitle(name);
    updateMenus();
    ui->projectDockWidget->setVisible(false);

    CodeEditor *editor = new CodeEditor;
    m_highlighter = new LanguageHighlighter(editor->document(), m_project->getUgens());
    m_highlighter->setDocument(editor->document()); // Not sure why, but this is required for highlighter to work.

    ui->tabWidget->addTab(editor, "code");

    if (!m_codeFile.open(QIODevice::ReadWrite)) {
        qDebug() << "Error opening code file!";
        throw;
    }
    setEditorText(m_codeFile.readAll());
}

ProjectWindow::~ProjectWindow()
{
    delete static_cast<SimpleProject *>(m_project);
    delete ui;
}

void ProjectWindow::build()
{
    ui->consoleText->clear();
    m_project->build();

}

void ProjectWindow::flash()
{
    ui->consoleText->clear();
    m_project->flash();
}

void ProjectWindow::run(bool pressed)
{
    if (pressed) {
        ui->consoleText->clear();
    }
    m_project->run(pressed);
}

void ProjectWindow::programStopped()
{
    ui->actionRun->setChecked(false);
}

void ProjectWindow::setTargetFromMenu()
{
    QAction *act = static_cast<QAction *>(sender());
    Q_ASSERT(act);

    m_project->setTarget(act->text());
}

void ProjectWindow::printConsoleText(QString text)
{
    ui->consoleText->setTextColor(Qt::black);
    ui->consoleText->append(text);

}

void ProjectWindow::printConsoleError(QString text)
{
    ui->consoleText->setTextColor(Qt::red);
    ui->consoleText->append(text);
}

void ProjectWindow::updateMenus()
{
    QStringList deviceList = m_project->listDevices();
    QStringList targetList = m_project->listTargets();

    ui->menuTarget->clear();
    QActionGroup *deviceGroup = new QActionGroup(ui->menuTarget);
    foreach (QString device, deviceList) {
        QAction *act = ui->menuTarget->addAction(device);
        act->setCheckable(true);
        deviceGroup->addAction(act);
        if (act->text().startsWith(m_project->getBoardId())) {
            act->setChecked(true);
        }
//        connect(act, SIGNAL(triggered()),
//                this, SLOT(setTargetFromMenu()));
    }
    ui->menuTarget->addSeparator();
    QActionGroup *targetGroup = new QActionGroup(ui->menuTarget);
    foreach (QString target, targetList) {
        QAction *act = ui->menuTarget->addAction(target);
        act->setCheckable(true);
        targetGroup->addAction(act);
        if (act->text() == m_project->getTarget()) {
            act->setChecked(true);
        }
        connect(act, SIGNAL(triggered()),
                this, SLOT(setTargetFromMenu()));
    }
}

void ProjectWindow::setEditorText(QString code)
{
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    editor->setPlainText(code);
}

void ProjectWindow::saveProject()
{
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    QString code = editor->toPlainText();
    m_codeFile.reset();
    m_codeFile.write(code.toLocal8Bit());
}

void ProjectWindow::connectActions()
{
    connect(ui->actionNew_Project, SIGNAL(triggered()), parent(), SLOT(newProject()));
    connect(ui->actionBuild, SIGNAL(triggered()), this, SLOT(build()));
    connect(ui->actionUpload, SIGNAL(triggered()), this, SLOT(flash()));
    connect(ui->actionRun, SIGNAL(toggled(bool)), this, SLOT(run(bool)));
    connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(updateMenus()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveProject()));

    connect(m_project, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
    connect(m_project, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
    connect(m_project, SIGNAL(programStopped()), this, SLOT(programStopped()));
}
