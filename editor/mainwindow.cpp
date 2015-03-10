#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectwindow.h"
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->projectTreeWidget->addTopLevelItem(
                new QTreeWidgetItem(QStringList() << "Recent Projects", QTreeWidgetItem::Type));
    ui->projectTreeWidget->addTopLevelItem(
                new QTreeWidgetItem(QStringList() << "Example Projects", QTreeWidgetItem::Type));
    connectActions();
    m_baseProjectDir = QDir::homePath() + "/StreamStack";
    QDir dir(m_baseProjectDir);
    if (!QFile::exists(m_baseProjectDir)) {
        qDebug() << "Creating base project directory at " << m_baseProjectDir;
        dir.mkpath(m_baseProjectDir);
    }
    setWindowTitle("StreamStacker");
    newProject();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newProject()
{
    ProjectWindow *pw = new ProjectWindow(this, m_baseProjectDir);
    pw->newFile();
//    pw->setWindowFlags(pw->windowFlags() | Qt::Window);
    pw->show();
}

void MainWindow::loadProject()
{
    // TODO should there be an actual project file to load? Having to select a directory is unintuitive
    QString path = QFileDialog::getExistingDirectory(this);
    if(!path.isEmpty()) {
        try {
            ProjectWindow *pw = new ProjectWindow(this, path);
//            pw->setWindowFlags(pw->windowFlags() | Qt::Window);
            pw->show();
        }
        catch (...) {
            QMessageBox::critical(this, tr("Project Error"), tr("Error opening project."));
        }
    }
}

void MainWindow::connectActions()
{
    connect(ui->actionNew_project, SIGNAL(triggered()), this, SLOT(newProject()));
    connect(ui->actionLoad_Project, SIGNAL(triggered()), this, SLOT(loadProject()));
}

