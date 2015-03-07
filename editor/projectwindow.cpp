#include "projectwindow.h"
#include "ui_projectwindow.h"

#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QMenuBar>
#include <QSettings>
#include <QTemporaryFile>

#include "codeeditor.h"
//#include "xmosproject.h"
#include "ast.h"
#include "codegen.h"
#include "configdialog.h"

ProjectWindow::ProjectWindow(QWidget *parent, QString projectDir) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    m_codeFile(projectDir + "/code/code.st"),
    m_projectDir(projectDir),
    m_timer(this)
{
    ui->setupUi(this);

    m_platformsRootDir = "../../StreamStack/platforms";
    QString xmosToolChainRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";
//    m_project = new XmosProject(projectDir, platformsRootDir, xmosToolChainRoot);
//    m_project->setBoardId("0ontZocni8POZ");

    connectActions();

    QString name = projectDir.mid(projectDir.lastIndexOf("/") + 1);
    setWindowTitle(name);
    updateMenus();
    ui->projectDockWidget->setVisible(false);

    CodeEditor *editor = new CodeEditor;
    editor->setFilename(m_projectDir + "/code/code.st");
//    m_highlighter = new LanguageHighlighter(editor->document(), m_project->getUgens());
    m_highlighter = new LanguageHighlighter(editor->document(), NULL);
    m_highlighter->setDocument(editor->document()); // Not sure why, but this is required for highlighter to work.

    ui->tabWidget->addTab(editor, "code");

    if (!QFile::exists(projectDir)) {
        QDir().mkpath(projectDir);
        QDir().mkpath(projectDir + "/code");
    }

    if (!m_codeFile.open(QIODevice::ReadWrite)) { // ReadWrite creates the file if it doesn't exist
        qDebug() << "Error opening code file!";
        throw;
    }
    setEditorText(m_codeFile.readAll());
    m_codeFile.close();

    readSettings();

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateCodeAnalysis()));
    m_timer.start(2000);
}

ProjectWindow::~ProjectWindow()
{
//    delete static_cast<XmosProject *>(m_project);
    delete ui;
}

void ProjectWindow::build()
{
    ui->consoleText->clear();
//    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    saveProject();
//    m_project->setCode(editor->toPlainText());
//    m_project->build();
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    AST *tree;
    tree = AST::parseFile(editor->filename().toLocal8Bit().constData());
    Codegen generator(m_platformsRootDir, tree);
//    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    editor->setErrors(errors);

    foreach(LangError error, errors) {
        ui->consoleText->insertPlainText(error.getErrorText() + "\n");
    }
    if (tree) {
        delete tree;
    }
}

void ProjectWindow::flash()
{
    ui->consoleText->clear();
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
//    m_project->setCode(editor->toPlainText());
//    m_project->flash();
}

void ProjectWindow::run(bool pressed)
{
    if (pressed) {
        ui->consoleText->clear();
    }
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
//    m_project->setCode(editor->toPlainText());
//    m_project->run(pressed);
}

void ProjectWindow::programStopped()
{
    ui->actionRun->setChecked(false);
}

void ProjectWindow::setTargetFromMenu()
{
    QAction *act = static_cast<QAction *>(sender());
    Q_ASSERT(act);

//    m_project->setTarget(act->text());
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

//void ProjectWindow::createMenus()
//{
//    QMenu *fileMenu = new QMenu("File");
//    fileMenu->addAction(tr("New"),parent(), SLOT(newProject()));

//    //    connect(ui->actionNew_Project, SIGNAL(triggered()), );
//    //    connect(ui->actionLoad, SIGNAL(triggered()), parent(), SLOT(loadProject()));
//        //    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveProject()));
//}

void ProjectWindow::updateMenus()
{
//    QStringList deviceList = m_project->listDevices();
//    QStringList targetList = m_project->listTargets();

//    ui->menuTarget->clear();
//    QActionGroup *deviceGroup = new QActionGroup(ui->menuTarget);
//    foreach (QString device, deviceList) {
//        QAction *act = ui->menuTarget->addAction(device);
//        act->setCheckable(true);
//        deviceGroup->addAction(act);
//        if (act->text().startsWith(m_project->getBoardId())) {
//            act->setChecked(true);
//        }
////        connect(act, SIGNAL(triggered()),
////                this, SLOT(setTargetFromMenu()));
//    }
//    ui->menuTarget->addSeparator();
//    QActionGroup *targetGroup = new QActionGroup(ui->menuTarget);
//    foreach (QString target, targetList) {
//        QAction *act = ui->menuTarget->addAction(target);
//        act->setCheckable(true);
//        targetGroup->addAction(act);
//        if (act->text() == m_project->getTarget()) {
//            act->setChecked(true);
//        }
//        connect(act, SIGNAL(triggered()),
//                this, SLOT(setTargetFromMenu()));
//    }
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
    if (!m_codeFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Error opening code file for writing!";
        throw;
    }
    m_codeFile.write(code.toLocal8Bit());
    m_codeFile.close();
}

void ProjectWindow::openOptionsDialog()
{
    ConfigDialog config(this);
    config.setFont(
                QFont(m_options["editor.fontFamily"].toString(),
                      m_options["editor.fontSize"].toFloat(),
            m_options["editor.fontWeight"].toInt(),
            m_options["editor.fontItalic"].toBool()
            )
            );

    config.setHighlighterFormats(m_highlighter->formats());

    int result = config.exec();
    if (result == QDialog::Accepted) {
        // TODO apply to all editors
        QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
        QFont font = config.font();
        editor->setFont(font);
        m_options["editor.fontFamily"] = font.family();
        m_options["editor.fontSize"] = font.pointSizeF();
        m_options["editor.fontWeight"] = font.weight();
        m_options["editor.fontItalic"] = font.italic();
        m_highlighter->setFormats(config.highlighterFormats());
        writeSettings();
    }
}

void ProjectWindow::updateCodeAnalysis()
{
    if (QApplication::activeWindow() == this) {
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
        QTemporaryFile tmpFile;
        if (tmpFile.open()) {
            tmpFile.write(editor->document()->toPlainText().toLocal8Bit());
            tmpFile.close();
            AST *tree;
            tree = AST::parseFile(tmpFile.fileName().toLocal8Bit().constData());
            Codegen generator(m_platformsRootDir, tree);
            //    QVERIFY(!generator.isValid());
            QList<LangError> errors = generator.getErrors();

            editor->setErrors(errors);
            if (tree) {
                delete tree;
            }
        }
    }
}

void ProjectWindow::connectActions()
{

    connect(ui->actionNew_Project, SIGNAL(triggered()), parent(), SLOT(newProject()));
    connect(ui->actionBuild, SIGNAL(triggered()), this, SLOT(build()));
    connect(ui->actionUpload, SIGNAL(triggered()), this, SLOT(flash()));
    connect(ui->actionRun, SIGNAL(toggled(bool)), this, SLOT(run(bool)));
    connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(updateMenus()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openOptionsDialog()));

//    connect(ui->buildButton, SIGNAL(clicked()), this, SLOT(build()));
//    connect(ui->uploadButton, SIGNAL(clicked()), this, SLOT(flash()));
//    connect(ui->runButton, SIGNAL(toggled(bool)), this, SLOT(run(bool)));
//    connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(updateMenus()));

//    connect(m_project, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
//    connect(m_project, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
    //    connect(m_project, SIGNAL(programStopped()), this, SLOT(programStopped()));
}

void ProjectWindow::readSettings()
{
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    QSettings settings("StreamStack", "StreamStackEdit", this);
    settings.beginGroup("project");

    QString fontFamily = settings.value("editor.fontFamily", "Courier").toString();
    qreal fontSize = settings.value("editor.fontSize", 10.0).toFloat();
    int fontWeight = settings.value("editor.fontSize", QFont::Normal).toInt();
    bool fontItalic = settings.value("editor.fontItalic", false).toBool();
    QFont font(fontFamily, fontSize, fontWeight, fontItalic);
    font.setPointSizeF(fontSize);
    editor->setFont(font);
    m_options["editor.fontFamily"] = fontFamily;
    m_options["editor.fontSize"] = font.pointSizeF();
    m_options["editor.fontWeight"] = font.weight();
    m_options["editor.fontItalic"] = font.italic();

    settings.endGroup();
    settings.beginGroup("highlighter");
    QMap<QString, QTextCharFormat> formats;
    QStringList keys = settings.childGroups();
    foreach(QString key, keys) {
        settings.beginGroup(key);
        QTextCharFormat format;
        format.setForeground(settings.value("foreground").value<QBrush>());
        format.setBackground(settings.value("background").value<QBrush>());
        format.setFontWeight(settings.value("bold").toInt());
        format.setFontItalic(settings.value("italic").toBool());
        settings.endGroup();
        formats[key] = format;
    }
    if (!formats.isEmpty()) {
        m_highlighter->setFormats(formats);
    }
    settings.endGroup();
}

void ProjectWindow::writeSettings()
{
    QSettings settings("StreamStack", "StreamStackEdit", this);
    settings.beginGroup("project");
    QStringList keys = m_options.keys();
     foreach(QString key, keys) {
         settings.setValue(key, m_options[key]);
     }
     settings.endGroup();
     settings.beginGroup("highlighter");
     QMapIterator<QString, QTextCharFormat> i(m_highlighter->formats());
     while(i.hasNext()) {
         i.next();
         settings.beginGroup(i.key());
         QTextCharFormat format = i.value();
         settings.setValue("foreground", format.foreground());
         settings.setValue("background", format.background());
         settings.setValue("bold", format.fontWeight());
         settings.setValue("italic", format.fontItalic());
         settings.endGroup();
     }
     settings.endGroup();
}
