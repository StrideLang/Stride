
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QMenuBar>
#include <QSettings>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QFileInfo>

#include "projectwindow.h"
#include "ui_projectwindow.h"

#include "codeeditor.h"
//#include "xmosproject.h"
#include "ast.h"
#include "codevalidator.h"
#include "configdialog.h"
#include "savechangeddialog.h"

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    m_timer(this),
    m_project(NULL)
{
    ui->setupUi(this);

    m_platformsRootDir = "../../StreamStack/platforms";
//    QString xmosToolChainRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";
//    m_project = new XmosProject(projectDir, platformsRootDir, xmosToolChainRoot);
//    m_project->setBoardId("0ontZocni8POZ");

    connectActions();

    setWindowTitle("StreamStack");
    updateMenus();
    ui->projectDockWidget->setVisible(false);
    m_highlighter = new LanguageHighlighter(this);

    readSettings();

    if (ui->tabWidget->count() == 0) {
        newFile(); // No files from previous session
    }

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateCodeAnalysis()));
    m_timer.start(2000);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    QTimer::singleShot(200, this, SLOT(updateCodeAnalysis()));
}

ProjectWindow::~ProjectWindow()
{
    delete ui;
}

void ProjectWindow::build()
{
    ui->consoleText->clear();
    saveFile();
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    AST *tree;
    tree = AST::parseFile(editor->filename().toLocal8Bit().constData());
//    tree = AST::parseFile(editor->filename().toStdString().c_str());
    CodeValidator validator(m_platformsRootDir, tree);
    QList<LangError> errors = validator.getErrors();
    editor->setErrors(errors);

    foreach(LangError error, errors) {
        ui->consoleText->insertPlainText(error.getErrorText() + "\n");
        return;
    }
    if (tree) {
        if (m_project) {
            delete m_project;
        }
        QString projectDir = makeProjectForCurrent();
        QString pythonExec = "python";
        m_project = new PythonProject(this, tree, validator.getPlatform(), projectDir, pythonExec);
        m_project->build();
        delete tree;
    }
    //    m_project->build();
}

void ProjectWindow::flash()
{
    ui->consoleText->clear();
}

void ProjectWindow::run(bool pressed)
{
    //    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (pressed) {
        build();
        ui->consoleText->clear();
        m_project->run();
    } else {
        m_project->stopRunning();
    }
}

void ProjectWindow::stop()
{
    if (m_project) {
        m_project->stopRunning();
    }
    ui->actionRun->setChecked(false);
}

void ProjectWindow::tabChanged(int index)
{
    Q_UNUSED(index);
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    m_highlighter->setDocument(editor->document());
}

bool ProjectWindow::maybeSave()
{
    QStringList modifiedFiles;
    QList<int> modifiedFilesTabs;
    for(int i = 0; i < ui->tabWidget->count(); i++) {
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
        if(editor->isChanged()) {
            modifiedFiles << editor->filename();
            modifiedFilesTabs << i;
        }
    }

    if(modifiedFiles.isEmpty()) {
        return true;
    }
    SaveChangedDialog d(this);
    d.setListContents(modifiedFiles);

    int but = d.exec();
    if (but == QDialog::Accepted) {
        foreach(int index, d.getSelected()) {
            saveFile(modifiedFilesTabs[index]);
        }
        return true;
    } else {
        return false;
    }
}

void ProjectWindow::programStopped()
{
    ui->actionRun->setChecked(false);
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

void ProjectWindow::saveFile(int index)
{
    CodeEditor *editor;
    if (index == -1) {
        editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    } else {
        editor = static_cast<CodeEditor *>(ui->tabWidget->widget(index));
    }
    Q_ASSERT(editor);
    if (editor->filename().isEmpty()) {
        if (!saveFileAs()) {
            return;
        }
    }
    QString code = editor->toPlainText();
    QFile codeFile(editor->filename());
    if (!codeFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Error opening code file for writing!";
        throw;
    }
    editor->markChanged(false);
    codeFile.write(code.toLocal8Bit());
    codeFile.close();
}

bool ProjectWindow::saveFileAs()
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file as:"));
    if  (fileName.isEmpty()) {
        return false;
    }
    QFile codeFile(fileName);
    if (!codeFile.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, tr("Error writing file"),
                              tr("Can't open file for writing."));
        return false;
    }
    editor->setFilename(fileName);
    editor->markChanged(false);
    codeFile.close();
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                              QFileInfo(fileName).fileName());
    return true;
}

void ProjectWindow::loadFile()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("LoadFile"));
    if (!fileNames.isEmpty()) {
        foreach(QString fileName, fileNames) {
            loadFile(fileName);
        }
    }
}

void ProjectWindow::loadFile(QString fileName)
{
    newFile();
    QFile codeFile(fileName);
    if (!codeFile.open(QIODevice::ReadWrite)) { // ReadWrite creates the file if it doesn't exist
        qDebug() << "Error opening code file!";
        return;
    }
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    setEditorText(codeFile.readAll());
    editor->setFilename(QFileInfo(fileName).absoluteFilePath());
    editor->markChanged(false);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                              QFileInfo(fileName).fileName());
    codeFile.close();
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

    QMap<QString, QTextCharFormat> formats = m_highlighter->formats();
    config.setHighlighterFormats(formats);

    connect(&config, SIGNAL(requestHighlighterPreset(int)),
            m_highlighter, SLOT(setFormatPreset(int)));
    connect(m_highlighter, SIGNAL(currentHighlightingChanged(QMap<QString,QTextCharFormat> &)),
            &config, SLOT(setHighlighterFormats(QMap<QString,QTextCharFormat> &)));

    int result = config.exec();
    if (result == QDialog::Accepted) {
        m_font = config.font();
        updateEditorFont();
        m_options["editor.fontFamily"] = m_font.family();
        m_options["editor.fontSize"] = m_font.pointSizeF();
        m_options["editor.fontWeight"] = m_font.weight();
        m_options["editor.fontItalic"] = m_font.italic();
        m_highlighter->setFormats(config.highlighterFormats());
        writeSettings();
    }
}

void ProjectWindow::updateCodeAnalysis()
{
    if (QApplication::activeWindow() == this) { // FIXME check if document has been modified to avoid doing this unnecessarily
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
        QTemporaryFile tmpFile;
        if (tmpFile.open()) {
            tmpFile.write(editor->document()->toPlainText().toLocal8Bit());
            tmpFile.close();
            AST *tree;
            tree = AST::parseFile(tmpFile.fileName().toLocal8Bit().constData());

            if (tree) {
                CodeValidator validator(m_platformsRootDir, tree);
                //    QVERIFY(!generator.isValid());
                QStringList types = validator.getPlatform().getPlatformTypes();
                m_highlighter->setBlockTypes(types);
                QStringList funcs = validator.getPlatform().getFunctions();
                m_highlighter->setFunctions(funcs);
                QList<PlatformObject> objects = validator.getPlatform().getBuiltinObjects();
                QStringList objectNames;
                foreach (PlatformObject platObject, objects) {
                    objectNames << platObject.getName();
                }
                m_highlighter->setBuiltinObjects(objectNames);
                QList<LangError> errors = validator.getErrors();
                editor->setErrors(errors);
                delete tree;
            }
        }
    }
}

void ProjectWindow::connectActions()
{

    connect(ui->actionNew_Project, SIGNAL(triggered()), this, SLOT(newFile()));
    connect(ui->actionBuild, SIGNAL(triggered()), this, SLOT(build()));
    connect(ui->actionUpload, SIGNAL(triggered()), this, SLOT(flash()));
    connect(ui->actionRun, SIGNAL(toggled(bool)), this, SLOT(run(bool)));
    connect(ui->actionRefresh, SIGNAL(triggered()), this, SLOT(updateMenus()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openOptionsDialog()));
    connect(ui->actionLoad_File, SIGNAL(triggered()), this, SLOT(loadFile()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

//    connect(m_project, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
//    connect(m_project, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
    //    connect(m_project, SIGNAL(programStopped()), this, SLOT(programStopped()));
}

void ProjectWindow::readSettings()
{
    QSettings settings("StreamStack", "StreamStackEdit", this);
    settings.beginGroup("project");

    QString fontFamily = settings.value("editor.fontFamily", "Courier").toString();
    qreal fontSize = settings.value("editor.fontSize", 10.0).toFloat();
    int fontWeight = settings.value("editor.fontSize", QFont::Normal).toInt();
    bool fontItalic = settings.value("editor.fontItalic", false).toBool();
    m_font = QFont(fontFamily, fontSize, fontWeight, fontItalic);
    m_font.setPointSizeF(fontSize);
    updateEditorFont();
    m_options["editor.fontFamily"] = fontFamily;
    m_options["editor.fontSize"] = m_font.pointSizeF();
    m_options["editor.fontWeight"] = m_font.weight();
    m_options["editor.fontItalic"] = m_font.italic();

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

    settings.beginGroup("GUI");
    int size = settings.beginReadArray("openDocuments");
    for(int i = 0; i < ui->tabWidget->count(); i++) {
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
        settings.setArrayIndex(i);
        settings.setValue("ServerItem", editor->filename());
    }
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString fileName = settings.value("fileName").toString();
        if (fileName.isEmpty()) {
            newFile();
        } else {
            if (QFile::exists(fileName)) {
                loadFile(fileName);
            } else {
                QMessageBox::warning(this, tr("File not found"),
                                     tr("Previously open file %1 not found.").arg(fileName));
            }
        }
    }
    settings.endArray();
    ui->tabWidget->setCurrentIndex(settings.value("lastIndex", -1).toInt());
    tabChanged(settings.value("lastIndex", -1).toInt()); // Should be triggered automatically but isn't...
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

     settings.beginGroup("GUI");
     settings.setValue("lastIndex", ui->tabWidget->currentIndex());
     settings.beginWriteArray("openDocuments");
     for(int i = 0; i < ui->tabWidget->count(); i++) {
         CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
         settings.setArrayIndex(i);
         settings.setValue("fileName", editor->filename());
     }
     settings.endArray();

     settings.endGroup();
}

void ProjectWindow::updateEditorFont()
{
    for(int i = 0; i < ui->tabWidget->count(); i++) {
        QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->widget(i));
        editor->setFont(m_font);

        const int tabStop = 4;  // 4 characters
        QFontMetrics metrics(m_font);
        editor->setTabStopWidth(tabStop * metrics.width(' '));
    }
}


QString ProjectWindow::makeProjectForCurrent()
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    Q_ASSERT(!editor->filename().isEmpty());
    QFileInfo info(editor->filename());
    QString dirName = info.absolutePath() + QDir::separator()
            + info.fileName() + "_Products";
    if (!QFile::exists(dirName)) {
        if (!QDir().mkpath(dirName)) {
            qDebug() << "Error creating project path";
            return QString();
        }
    }
    return dirName;
}

void ProjectWindow::newFile()
{
    // Create editor tab
    CodeEditor *editor = new CodeEditor(this);
    editor->setFilename("");
//    m_highlighter = new LanguageHighlighter(editor->document(), m_project->getUgens());
    int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex() + 1, editor, "untitled");
    ui->tabWidget->setCurrentIndex(index);
    updateEditorFont();
    m_highlighter->setDocument(editor->document());
}

void ProjectWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        if (m_project) {
            m_project->stopRunning();
            delete m_project;
        }
        event->accept();
    } else {
        event->ignore();
    }
}
