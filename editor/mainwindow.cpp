#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "projectwindow.h"
#include <QDir>
#include <QDebug>
#include <QFileDialog>

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
    QString projectDir = m_baseProjectDir + "/untitled_00";
    int count = 1;
    while (QFile::exists(projectDir)) {
        projectDir = projectDir.left(projectDir.lastIndexOf('_') + 1)
                + QString("%1").arg(count, 2, 10, QChar('0'));
        count++;
        if (count == 999999) {
            qDebug() << "Warning! Too many untitled projects, overwriting.";
            break;
        }
    }

    ProjectWindow *pw = new ProjectWindow(this, projectDir);
    pw->show();
}

void MainWindow::loadProject()
{
    QString path = QFileDialog::getExistingDirectory(this);
    ProjectWindow *pw = new ProjectWindow(this, path);
    pw->show();
}

void MainWindow::connectActions()
{
    connect(ui->actionNew_project, SIGNAL(triggered()), this, SLOT(newProject()));
    connect(ui->actionLoad_Project, SIGNAL(triggered()), this, SLOT(loadProject()));
}

