
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

#include "pythonproject.h"

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    m_timer(this),
    m_builder(NULL)
{
    ui->setupUi(this);

    m_platformsRootDir = "../../StreamStack/platforms";
//    QString xmosToolChainRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";
//    m_project = new XmosProject(projectDir, platformsRootDir, xmosToolChainRoot);
//    m_project->setBoardId("0ontZocni8POZ");

    connectActions();
    connectShortcuts();

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
    ui->tabWidget->setDocumentMode(true);
    ui->tabWidget->setTabsClosable(true);
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(tabChanged(int)));

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
    QList<LangError> errors;
    tree = AST::parseFile(editor->filename().toLocal8Bit().constData());
//    tree = AST::parseFile(editor->filename().toStdString().c_str());
    if (!tree) {
        vector<LangError> syntaxErrors = AST::getParseErrors();
        for (unsigned int i = 0; i < syntaxErrors.size(); i++) {
            errors << syntaxErrors[i];
        }
    }
    CodeValidator validator(m_platformsRootDir, tree);
    errors << validator.getErrors();
    editor->setErrors(errors);

    foreach(LangError error, errors) {
        ui->consoleText->insertPlainText(QString::fromStdString(error.getErrorText()) + "\n");
        return;
    }
    if (tree) {
        if (m_builder) {
            delete m_builder;
        }
        StreamPlatform *platform = validator.getPlatform();
        QString projectDir = makeProjectForCurrent();
        m_builder = platform->createBuilder(projectDir);
        connect(m_builder, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
        connect(m_builder, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
        if (m_builder) {
            m_builder->build(tree);
        } else {
            qDebug() << "Can't create builder";
            Q_ASSERT(false);
        }
        delete tree;
    }
    //    m_project->build();
}

void ProjectWindow::commentSection()
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    QString commentChar = "#";
    QTextCursor cursor = editor->textCursor();
    int oldPosition = cursor.position();
    int oldAnchor = cursor.anchor();
    if (cursor.position() > cursor.anchor()) {
        int temp = cursor.anchor();
        cursor.setPosition(cursor.position());
        cursor.setPosition(temp, QTextCursor::KeepAnchor);
    }
    if (!cursor.atBlockStart()) {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    }
    QString text = cursor.selectedText();
    if (text.startsWith(commentChar)) {
        uncomment();
        return;
    }
    text.prepend(commentChar);
    text.replace(QChar(QChar::ParagraphSeparator), QString("\n" + commentChar));
    int numComments = text.count('\n');
    if (text.endsWith("\n" + commentChar) ) {
        text.chop(1);
        numComments--;
    }
    cursor.insertText(text);
    if (oldPosition <= oldAnchor) {
        cursor.setPosition(oldPosition);
    } else {
        cursor.setPosition(oldPosition + numComments);
    }
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, oldAnchor - oldPosition + numComments);
    editor->setTextCursor(cursor);
}

void ProjectWindow::uncomment()
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    QString commentChar = "#";
    QTextCursor cursor = editor->textCursor();
    int oldPosition = cursor.position();
    int oldAnchor = cursor.anchor();
    if (cursor.position() > cursor.anchor()) {
        int temp = cursor.anchor();
        cursor.setPosition(cursor.position());
        cursor.setPosition(temp, QTextCursor::KeepAnchor);
    }
    QString text = cursor.selectedText();
    if (!cursor.atBlockStart() && !text.startsWith(commentChar)) {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        text = cursor.selectedText();
    }
    int numComments = text.count('\n');
    if (text.startsWith(commentChar)) {
        text.remove(0,1);
        numComments--;
    }
    text.replace(QChar(QChar::ParagraphSeparator), QString("\n"));
    text.replace(QString("\n" + commentChar), QString("\n")); //TODO make more robust
    cursor.insertText(text);
    cursor.setPosition(oldPosition);
    if (oldPosition > oldAnchor) {
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, oldAnchor - oldPosition - numComments);
    } else {
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, oldPosition - oldAnchor - numComments);
    }
    editor->setTextCursor(cursor);
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
        Q_ASSERT(m_builder);
        ui->consoleText->clear();
        m_builder->run();
    } else {
        disconnect(m_builder, 0,0,0);
        m_builder->run(false);
        delete m_builder;
        m_builder = NULL;
    }
}

void ProjectWindow::stop()
{
    if (m_builder) {
        m_builder->run(false);
    }
    // For some reason setChecked triggers the run action the wrong way... so need to disbale these signals
    ui->actionRun->blockSignals(true);
    ui->actionRun->setChecked(false);
    ui->actionRun->blockSignals(false);
}

void ProjectWindow::tabChanged(int index)
{
    Q_UNUSED(index);
    if (index >= 0) {
        QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
        m_highlighter->setDocument(editor->document());
    }
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
    } else if (but == 100) { // Custom code for "don't save
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
    if (!text.isEmpty()) {
        ui->consoleText->setTextColor(Qt::black);
        ui->consoleText->append(text);
    }
}

void ProjectWindow::printConsoleError(QString text)
{
    if (!text.isEmpty()) {
        ui->consoleText->setTextColor(Qt::red);
        ui->consoleText->append(text);
    }
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
    QFile codeFile(fileName);
    if (!codeFile.open(QIODevice::ReadOnly)) { // ReadWrite creates the file if it doesn't exist
        qDebug() << "Error opening code file!";
        return;
    }
    newFile();
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    setEditorText(codeFile.readAll());
    editor->setFilename(QFileInfo(fileName).absoluteFilePath());
    editor->markChanged(false);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                              QFileInfo(fileName).fileName());
    codeFile.close();
    markModified();
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
                QStringList types = validator.getPlatform()->getPlatformTypeNames();
                m_highlighter->setBlockTypes(types);
                QStringList funcs = validator.getPlatform()->getFunctionNames();
                m_highlighter->setFunctions(funcs);
                QList<PlatformObject> objects = validator.getPlatform()->getBuiltinObjects();
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
    connect(ui->actionComment, SIGNAL(triggered(bool)), this, SLOT(commentSection()));
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

void ProjectWindow::connectShortcuts()
{
    ui->actionNew_Project->setShortcut(QKeySequence("Ctrl+N"));
    ui->actionBuild->setShortcut(QKeySequence("Ctrl+B"));
    ui->actionComment->setShortcut(QKeySequence("Ctrl+/"));
    ui->actionUpload->setShortcut(QKeySequence("Ctrl+U"));;
    ui->actionRun->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionRefresh->setShortcut(QKeySequence(""));
    ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionOptions->setShortcut(QKeySequence(""));
    ui->actionLoad_File->setShortcut(QKeySequence("Ctrl+O"));
    ui->actionStop->setShortcut(QKeySequence("Ctrl+Space"));
    ui->actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
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
    QMap<QString, QTextCharFormat> formats = m_highlighter->formats();
    QStringList keys = settings.childGroups();
    foreach(QString key, keys) {
        settings.beginGroup(key);
        QTextCharFormat format;
        format.setForeground(settings.value("foreground").value<QBrush>());
        format.setBackground(settings.value("background").value<QBrush>());
        format.setFontWeight(settings.value("bold").toInt());
        format.setFontItalic(settings.value("italic").toBool());
        settings.endGroup();
        if (formats.contains(key)) {
            formats[key] = format;
        }
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
    if (settings.value("lastIndex", -1).toInt() >= 0 && ui->tabWidget->count() > 0) {
        tabChanged(settings.value("lastIndex", -1).toInt()); // Should be triggered automatically but isn't...
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

    int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex() + 1, editor, "untitled");
    ui->tabWidget->setCurrentIndex(index);
    updateEditorFont();
    m_highlighter->setDocument(editor->document());
    QObject::connect(editor, SIGNAL(textChanged()), this, SLOT(markModified()));
}

void ProjectWindow::closeTab(int index)
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    qDebug() << "Close";
    ui->tabWidget->removeTab(index);
    delete editor;
}

void ProjectWindow::markModified()
{
    int currentIndex = ui->tabWidget->currentIndex();
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    QColor textColor;
    if (editor->document()->isModified()) {
        textColor = Qt::red;
    } else {
        QPalette p = this->palette();
        textColor = p.color(QPalette::Foreground);
    }
    ui->tabWidget->tabBar()->setTabTextColor(currentIndex, textColor);
}

void ProjectWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        if (m_builder) {
            m_builder->run(false);
            delete m_builder;
        }
        event->accept();
    } else {
        event->ignore();
    }
}
