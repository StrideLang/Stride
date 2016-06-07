
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
#include <QSignalMapper>
#include <QDesktopServices>

#include "projectwindow.h"
#include "ui_projectwindow.h"

#include "codeeditor.h"
#include "configdialog.h"
#include "savechangeddialog.h"

#include "pythonproject.h"

ProjectWindow::ProjectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProjectWindow),
    m_searchWidget(new SearchWidget(this)),
    m_codeModelTimer(this),
    m_builder(NULL),
    m_helperMenu(this),
    m_startingUp(true)
{
    ui->setupUi(this);

    connectActions();
    connectShortcuts();
    QIcon icon(":resources/icon.png");
    setWindowIcon(icon);

    setWindowTitle("StreamStack");
    updateMenus();
    ui->projectDockWidget->setVisible(false);
    m_highlighter = new LanguageHighlighter(this);

    readSettings();

    if (ui->tabWidget->count() == 0) {
        newFile(); // No files from previous session
    }

    ui->tabWidget->setDocumentMode(true);
    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->setMovable(true);
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(tabChanged(int)));

    m_startingUp = false;
    m_codeModelTimer.setSingleShot(true);
    m_codeModelTimer.setInterval(2000);
    connect(&m_codeModelTimer, SIGNAL(timeout()), this, SLOT(updateCodeAnalysis()));

    docBrowser = new QWebEngineView();
    ui->documentationDockWidget->setWidget(docBrowser);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_searchWidget.data());
    layout->setMargin(0);
    ui->belowEditorWidget->setLayout(layout);

    m_searchWidget->hide();
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
    m_codeModel.updateCodeAnalysis(editor->document()->toPlainText(),
                                   m_environment["platformRootPath"].toString());

    QList<LangError> errors = m_codeModel.getErrors();
    editor->setErrors(errors);

    foreach(LangError error, errors) {
        ui->consoleText->insertPlainText(QString::fromStdString(error.getErrorText()) + "\n");
    }
    if (errors.size() > 0) {
        return;
    }
    AST *optimizedTree = m_codeModel.getOptimizedTree();
    if (optimizedTree) {
        if (m_builder) {
            delete m_builder;
        }
        QString projectDir = makeProjectForCurrent();
        m_builder = m_codeModel.createBuilder(projectDir);
        if (m_builder) {
            connect(m_builder, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
            connect(m_builder, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
            m_builder->build(optimizedTree);
        } else {
            printConsoleText(tr("Done. No builder set."));
            qDebug() << "Can't create builder";
//            Q_ASSERT(false);
        }
        optimizedTree->deleteChildren();
        delete optimizedTree;
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
//        ui->consoleText->clear();
        if (m_builder) {
            m_builder->run();
        } else {
            ui->actionRun->setChecked(false);
            printConsoleError(tr("Can't run. No builder available."));
        }
    } else {
        stop();
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
        connect(editor, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showHelperMenu(QPoint)));
    }
}

bool ProjectWindow::maybeSave()
{
    QStringList modifiedFiles;
    QList<int> modifiedFilesTabs;
    for(int i = 0; i < ui->tabWidget->count(); i++) {
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
        if(editor->isChanged()) {
            QString name = editor->filename();
            if (name.isEmpty()) {
                name = "Untitled.stride";
            }
            modifiedFiles << name;
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

void ProjectWindow::showDocumentation()
{
    // TODO should all this be moved to the editor or related class?
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    QTextCursor cursor= editor->textCursor();
//    QTextDocument *doc = editor->document();
    if (cursor.selectedText() == "") {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    QString word = cursor.selectedText();
    QString html = m_codeModel.getHtmlDocumentation(word);
    if (html.isEmpty()) {
        html = tr("Unknown type: %1").arg(word);
    }
    docBrowser->setHtml(html);
}

void ProjectWindow::openGeneratedDir()
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    if(!editor->filename().isEmpty()){
        QFileInfo info(editor->filename());
        QString dirName = info.absolutePath() + QDir::separator()
                + info.fileName() + "_Products";
        QDesktopServices::openUrl(QUrl("file://" + dirName));
    }
}

void ProjectWindow::followSymbol()
{
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    QTextCursor cursor= editor->textCursor();
    if (cursor.selectedText() == "") {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    QString word = cursor.selectedText();
    QPair<QString, int> symbolLocation = m_codeModel.getSymbolLocation(word);
    QString fileName = symbolLocation.first;
    int lineNumber = symbolLocation.second;
    if (!fileName.isEmpty()) {
        loadFile(fileName);
        editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
        QTextCursor cursor(editor->document()->findBlockByLineNumber(lineNumber-1));
        editor->setTextCursor(cursor);
    }
}

void ProjectWindow::showHelperMenu(QPoint where)
{
    m_helperMenu.clear();

//    bool platformChosen = false;
//    foreach(AST *node, m_lastValidTree->getChildren()) {
//        if (node->getNodeType() == AST::Platform) {
//            platformChosen = true;
//            break;
//        }
//    }
    QMenu *platformMenu = m_helperMenu.addMenu(tr("Platform"));
    QStringList platformList, platformCode;
    platformList << "Gamma" << "Arduino";
    platformCode << "use Gamma version 1.0 on PC" << "use Arduino version 1.0 on Uno";
    for (int i = 0; i < platformList.size(); ++i) {
        QAction *newAction = platformMenu->addAction(platformList[i], this, SLOT(insertText()));
        newAction->setData(platformCode[i]);
    }
    QMenu *functionMenu = m_helperMenu.addMenu(tr("New function"));
    AST *optimizedTree = m_codeModel.getOptimizedTree();
    foreach(AST *node, optimizedTree->getChildren()) {
        if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            if (block->getObjectType() == "module") {
                QAction *newAction = functionMenu->addAction(QString::fromStdString(block->getName()), this, SLOT(insertText()));
                QString text = QString::fromStdString(block->getNamespace());
                if (!text.isEmpty()) {
                    text += ".";
                }
                text += QString::fromStdString(block->getName()) + "(";
                ListNode *portList = static_cast<ListNode *>(block->getPropertyValue("ports"));
                if (portList && portList->getNodeType() == AST::List) {
                    foreach(AST *port, portList->getChildren()) {
                        BlockNode *portBlock = static_cast<BlockNode *>(port);
                        AST *portName = portBlock->getPropertyValue("name");
                        if (portName && portName->getNodeType() == AST::String) {
                            text += QString::fromStdString(static_cast<ValueNode *>(portName)->getStringValue()) + ":  ";
                        }
                    }
                }
                text += ") ";
                newAction->setData(text);
            }
            node->deleteChildren();
            delete node;
        }
    }
    delete optimizedTree;

    m_helperMenu.exec(ui->tabWidget->currentWidget()->mapToGlobal(where));
}

void ProjectWindow::insertText(QString text)
{
    if (text.isEmpty()) {
        text = static_cast<QAction *>(sender())->data().toString();
    }

    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    editor->insertPlainText(text);
}

void ProjectWindow::find(QString query)
{
   m_searchWidget->show();
   QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
   QTextCursor cursor = editor->textCursor();
   if (query.isEmpty()) {
       cursor.select(QTextCursor::WordUnderCursor);
       editor->setTextCursor(cursor);
       query = cursor.selectedText();
   }
   m_searchWidget->setSearchString(query, true);
}

void ProjectWindow::findNext()
{
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    QString searchString = m_searchWidget.data()->searchString();
    QTextCursor cursor = editor->textCursor();
    if (cursor.selectedText() == searchString) {
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
    }
    if (!editor->find(searchString)) {
        // TODO not found or needs wrap
    }
}

void ProjectWindow::findPrevious()
{
    QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    QString searchString = m_searchWidget.data()->searchString();
    QTextCursor cursor = editor->textCursor();
    if (cursor.selectedText() == searchString) {
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    }
    if (!editor->find(searchString, QTextDocument::FindBackward)) {
        // TODO not found or needs wrap
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

bool ProjectWindow::saveFile(int index)
{
    CodeEditor *editor;
    if (index == -1) {
        editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    } else {
        editor = static_cast<CodeEditor *>(ui->tabWidget->widget(index));
    }
    Q_ASSERT(editor);
    if (editor->filename().isEmpty()) {
        return saveFileAs();
    }
    if (!editor->document()->isModified()) {
        return true;
    }
    QString code = editor->toPlainText();
    QFile codeFile(editor->filename());
    if (!codeFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Error opening code file for writing!";
//        throw;
    }
    editor->markChanged(false);
    codeFile.write(code.toLocal8Bit());
    codeFile.close();
    markModified();
    return true;
}

bool ProjectWindow::saveFileAs(int index)
{
    CodeEditor *editor;
    if (index == -1) {
        editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    } else {
        editor = static_cast<CodeEditor *>(ui->tabWidget->widget(index));
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file as:"));
    if  (fileName.isEmpty()) {
        return false;
    }
    if (!fileName.endsWith(".stride")) {
        fileName.append(".stride");
    }
    editor->setFilename(fileName);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                              QFileInfo(fileName).fileName());

    return saveFile(index);
}

void ProjectWindow::closeTab(int index)
{
    if (index < 0) {
        index = ui->tabWidget->currentIndex();
    }
    if (index >= ui->tabWidget->count()) {
        qDebug() << " ProjectWindow::closeTab(int index) invalid index " << index;
    }
    if (ui->tabWidget->count() > 1) {
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(index));
        if (editor->isChanged()) {
            QMessageBox::StandardButton result
                    = QMessageBox::question(this, tr("File modified"),
                                            tr("File has been modified. Save?"),
                                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                            QMessageBox::Yes);
            if (result == QMessageBox::Yes) {
                if (!saveFile()) {
                    return;
                }
            }
            if (result != QMessageBox::Cancel) {
                ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
                delete editor; // TODO better handling of this pointer (smarter pointer)
            }
        } else {
            ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
            delete editor;
        }
    }
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
    for(int i = 0; i < ui->tabWidget->count(); ++i) {
        CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
        if (editor->filename() == fileName) {
            ui->tabWidget->setCurrentWidget(editor);
            return;
        }
    }
    newFile();
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    setEditorText(codeFile.readAll());
    editor->setFilename(QFileInfo(fileName).absoluteFilePath());
    updateCodeAnalysis();
    editor->markChanged(false);
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                              QFileInfo(fileName).fileName());
    codeFile.close();
    markModified();
}

void ProjectWindow::openOptionsDialog()
{
    // First set current values
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

    config.setPlatformRootPath(m_environment["platformRootPath"].toString());


    // Connect
    connect(&config, SIGNAL(requestHighlighterPreset(int)),
            m_highlighter, SLOT(setFormatPreset(int)));
    connect(m_highlighter, SIGNAL(currentHighlightingChanged(QMap<QString,QTextCharFormat> &)),
            &config, SLOT(setHighlighterFormats(QMap<QString,QTextCharFormat> &)));

    int result = config.exec();
    // Get values back
    if (result == QDialog::Accepted) {
        m_font = config.font();
        updateEditorFont();
        m_options["editor.fontFamily"] = m_font.family();
        m_options["editor.fontSize"] = m_font.pointSizeF();
        m_options["editor.fontWeight"] = m_font.weight();
        m_options["editor.fontItalic"] = m_font.italic();
        m_highlighter->setFormats(config.highlighterFormats());

        m_environment["platformRootPath"] = config.platformRootPath();
        writeSettings();
    }
}

void ProjectWindow::updateCodeAnalysis()
{
    m_codeModelTimer.stop();
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    if ((QApplication::activeWindow() == this  && editor->document()->isModified())
            || m_startingUp) {
        m_codeModel.updateCodeAnalysis(editor->document()->toPlainText(),
                                       m_environment["platformRootPath"].toString());
        m_highlighter->setBlockTypes(m_codeModel.getTypes());
        m_highlighter->setFunctions(m_codeModel.getFunctions());
        m_highlighter->setBuiltinObjects(m_codeModel.getObjectNames());
        editor->setErrors(m_codeModel.getErrors());
    }
//    QTextCursor cursor = editor->textCursor();
//    cursor.select(QTextCursor::WordUnderCursor);
//    editor->setToolTipText(m_codeModel.getTooltipText(cursor.selectedText()));
    m_codeModelTimer.start();
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
    connect(ui->actionSave_as, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    connect(ui->actionClose_Tab, SIGNAL(triggered()), this, SLOT(closeTab()));
    connect(ui->actionOptions, SIGNAL(triggered()), this, SLOT(openOptionsDialog()));
    connect(ui->actionLoad_File, SIGNAL(triggered()), this, SLOT(loadFile()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionShow_Documentation, SIGNAL(triggered()),
            this, SLOT(showDocumentation()));
    connect(ui->actionFollow_Symbol, SIGNAL(triggered()), this, SLOT(followSymbol()));
    connect(ui->actionOpen_Generated_Code, SIGNAL(triggered()), this, SLOT(openGeneratedDir()));

    connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(find()));
    connect(ui->actionFind_Next, SIGNAL(triggered()), this, SLOT(findNext()));
    connect(ui->actionFind_Previous, SIGNAL(triggered()), this, SLOT(findPrevious()));

    connect(m_searchWidget.data()->getFindNextButton(), SIGNAL(released()),
            this, SLOT(findNext()));
    connect(m_searchWidget.data()->getFindPreviousButton(), SIGNAL(released()),
            this, SLOT(findPrevious()));


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
    if (settings.contains("geometry")) {
        this->restoreGeometry(settings.value("geometry").toByteArray());
    }
    int size = settings.beginReadArray("openDocuments");
    QStringList filesToOpen;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        filesToOpen << settings.value("fileName").toString();
    }
    settings.endArray();
    ui->tabWidget->setCurrentIndex(settings.value("lastIndex", -1).toInt());
    if (settings.value("lastIndex", -1).toInt() >= 0 && ui->tabWidget->count() > 0) {
        tabChanged(settings.value("lastIndex", -1).toInt()); // Should be triggered automatically but isn't...
    }
    settings.endGroup();

    settings.beginGroup("environment");
    m_environment["platformRootPath"] = settings.value("platformRootPath", "../../StreamStack/platforms").toString();
    settings.endGroup();

    foreach(QString fileName, filesToOpen) {
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
     settings.setValue("geometry", saveGeometry());
     settings.setValue("lastIndex", ui->tabWidget->currentIndex());
     settings.beginWriteArray("openDocuments");
     for(int i = 0; i < ui->tabWidget->count(); i++) {
         CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
         settings.setArrayIndex(i);
         settings.setValue("fileName", editor->filename());
     }
     settings.endArray();

     settings.endGroup();


     settings.beginGroup("environment");
     settings.setValue("platformRootPath", m_environment["platformRootPath"]);
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
    CodeEditor *editor = new CodeEditor(this, &m_codeModel);
    editor->setFilename("");

    int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex() + 1, editor, "untitled");
    ui->tabWidget->setCurrentIndex(index);
    updateEditorFont();
    m_highlighter->setDocument(editor->document());
    QObject::connect(editor, SIGNAL(textChanged()), this, SLOT(markModified()));
    editor->setFocus();
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
    //    QWidget::closeEvent(event);
}

bool ProjectWindow::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        loadFile(openEvent->file());
        return true;
    }
//    qDebug() << "Application event: " << event->type();
    return false;
}
