#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QProcess>
#include <QDebug>
#include <QQuickView>

#include "simpleproject.h"

ProjectWindow::ProjectWindow(QWidget *parent, QString projectDir) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow)
{
    ui->setupUi(this);

    m_project = new SimpleProject(projectDir);

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
    Q_ASSERT(false);
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

void ProjectWindow::connectActions()
{
    connect(ui->actionBuild, SIGNAL(triggered()), this, SLOT(build()));
    connect(ui->actionFlash, SIGNAL(triggered()), this, SLOT(flash()));
    connect(ui->actionSimple, SIGNAL(toggled(bool)), this, SLOT(setView(bool)));

    connect(m_project, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
    connect(m_project, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));

}
