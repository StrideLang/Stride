#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QProcess>
#include <QDebug>
#include <QQuickView>
#include <QActionGroup>

#include "simpleproject.h"

ProjectWindow::ProjectWindow(QWidget *parent, QString projectDir) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow)
{
    ui->setupUi(this);

    m_project = new SimpleProject(projectDir);
    m_project->setBoardId("0ontZocni8POZ");

    // Create QML Editor for layout panel
    QQuickView *view = new QQuickView();
    m_layoutContainer = QWidget::createWindowContainer(view, this);
    m_layoutContainer->setMinimumSize(200, 200);
//    m_layoutContainer->setMaximumSize(200, 200);
//    m_layoutContainer->setFocusPolicy(Qt::TabFocus);
    view->setSource(QUrl("qrc:/qml/Editor.qml"));
    ui->layoutDockWidget->setWidget(m_layoutContainer);

    connectActions();

    QString name = projectDir.mid(projectDir.lastIndexOf("/") + 1);
    setWindowTitle(name);
    updateMenus();

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

void ProjectWindow::setView(bool simple)
{

    ui->consoleDockWidget->setVisible(!simple);
    ui->projectDockWidget->setVisible(!simple);
    ui->documentationDockWidget->setVisible(!simple);
    ui->objectTabWidget->setVisible(!simple);
    ui->layoutDockWidget->setVisible(!simple);
    if (simple) {
//        ui->layoutDockWidget->setWidget(NULL);
        m_layoutContainer->setParent(ui->centralwidget);
        ui->verticalLayout_3->addWidget(m_layoutContainer);
//        m_layoutContainer->setVisible(true);
    } else {
        ui->layoutDockWidget->setWidget(m_layoutContainer);
    }
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

void ProjectWindow::connectActions()
{
    connect(ui->actionBuild, SIGNAL(triggered()), this, SLOT(build()));
    connect(ui->actionUpload, SIGNAL(triggered()), this, SLOT(flash()));
    connect(ui->actionRun, SIGNAL(toggled(bool)), this, SLOT(run(bool)));
    connect(ui->actionSimple, SIGNAL(toggled(bool)), this, SLOT(setView(bool)));

    connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(updateMenus()));

    connect(m_project, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
    connect(m_project, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
    connect(m_project, SIGNAL(programStopped()), this, SLOT(programStopped()));

}
