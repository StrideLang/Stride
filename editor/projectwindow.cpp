/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QSignalMapper>
#include <QWebEngineSettings>

// For config dialogs
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>

#include "projectwindow.h"
#include "ui_projectwindow.h"

#include "astfunctions.h"
#include "astquery.h"
#include "codeeditor.h"
#include "coderesolver.h"
#include "codevalidator.h"
#include "configdialog.h"
#include "localmanagementdialog.hpp"
#include "savechangeddialog.h"
#include "striderootmanagementdialog.h"

//#include "pythonproject.h"

ProjectWindow::ProjectWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ProjectWindow),
      m_searchWidget(new SearchWidget(this)), m_codeModelTimer(this),
      m_helperMenu(this), m_startingUp(true) {
  ui->setupUi(this);
  QIcon icon(":resources/icon.png");
  setWindowIcon(icon);
  setWindowTitle("StrideIDE");
}

ProjectWindow::~ProjectWindow() { delete ui; }

void ProjectWindow::initialize(bool resetOpenFiles) {
  connectActions();
  connectShortcuts();
  prepareMenus();
  m_highlighter = new LanguageHighlighter(this);

  readSettings(resetOpenFiles);

  if (ui->tabWidget->count() == 0) {
    newFile(); // No files from previous session
  }

  ui->tabWidget->setDocumentMode(true);
  ui->tabWidget->setTabsClosable(true);
  ui->tabWidget->setMovable(true);
  connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this,
          SLOT(closeTab(int)));
  connect(ui->tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(tabChanged(int)));

  m_startingUp = false;
  m_codeModelTimer.setSingleShot(true);
  m_codeModelTimer.setInterval(2000);
  connect(&m_codeModelTimer, SIGNAL(timeout()), this,
          SLOT(updateCodeAnalysis()));

  m_codeModelTimer.start();

  //    ui->documentationWidget->setHtml("<h1>Welcome to Stride</h1> A
  //    declarative and reactive domain specific programming language for
  //    real-time sound synthesis, processing, and interaction design. By Andrés
  //    Cabrera and Joseph Tilbian.");
  ui->documentationDockWidget->setFeatures(QDockWidget::DockWidgetClosable |
                                           QDockWidget::DockWidgetMovable |
                                           QDockWidget::DockWidgetFloatable);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(m_searchWidget.data());
  layout->setContentsMargins(0, 0, 0, 0);
  ui->belowEditorWidget->setLayout(layout);

  m_searchWidget->hide();

  ui->projectDockWidget->setVisible(true);
  connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
          this, SLOT(inspectorItemClicked(QTreeWidgetItem *, int)));
}

bool ProjectWindow::build() {
  bool buildOK = false;
  ui->consoleText->clear();
  saveFile();
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());

  QList<LangError> errors;
  std::vector<LangError> syntaxErrors;

  ASTNode tree;
  tree = ASTFunctions::parseFile(editor->filename().toLocal8Bit().constData());

  syntaxErrors = ASTFunctions::getParseErrors();

  if (syntaxErrors.size() > 0) {
    for (auto &syntaxError : syntaxErrors) {
      errors << syntaxError;
    }
    editor->setErrors(errors);

    foreach (LangError error, syntaxErrors) {
      printConsoleText(error.getErrorText() + "\n");
    }
    return false;
  }

  if (tree) {
    CodeEditor *editor =
        static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    SystemConfiguration systemConfig;
    systemConfig.readConfiguration(editor->filename().toStdString());

    CodeResolver resolver(
        tree, m_environment["striderootPath"].toString().toStdString(),
        systemConfig);
    resolver.process();
    CodeValidator validator(tree);
    for (auto err : validator.getErrors()) {
      errors.append(err);
    }

    if (errors.size() > 0) {
      editor->setErrors(errors);

      foreach (LangError error, errors) {
        printConsoleText(error.getErrorText() + "\n");
      }
      return false;
    }
    std::shared_ptr<StrideSystem> system = resolver.getSystem();

    for (auto builder : m_builders) {
      delete builder;
    }

    m_builders = system->createBuilders(editor->filename().toStdString(), tree);
    if (m_builders.size() == 0) {
      printConsoleText(tr("Aborting. No builder available.").toStdString());
      qDebug() << "Can't create builder";
      return false;
    }
    buildOK = true;

    system->generateDomainConnections(tree);

    for (auto builder : m_builders) {
      builder->m_system = system;
      builder->yieldCallback = []() { qApp->processEvents(); };
      builder->outputText = std::bind(&ProjectWindow::printConsoleText, this,
                                      std::placeholders::_1);
      builder->errorText = std::bind(&ProjectWindow::printConsoleText, this,
                                     std::placeholders::_1);
      builder->programStopped = std::bind(&ProjectWindow::programStopped, this);

      auto domainMap = builder->generateCode(tree);

      buildOK &= builder->build(domainMap);
    }
  }
  return buildOK;
}

void ProjectWindow::commentSection() {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
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
  if (text.endsWith("\n" + commentChar)) {
    text.chop(1);
    numComments--;
  }
  cursor.insertText(text);
  if (oldPosition <= oldAnchor) {
    cursor.setPosition(oldPosition);
  } else {
    cursor.setPosition(oldPosition + numComments);
  }
  cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                      oldAnchor - oldPosition + numComments);
  editor->setTextCursor(cursor);
}

void ProjectWindow::uncomment() {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
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
    text.remove(0, 1);
    numComments--;
  }
  text.replace(QChar(QChar::ParagraphSeparator), QString("\n"));
  text.replace(QString("\n" + commentChar),
               QString("\n")); // TODO make more robust
  cursor.insertText(text);
  cursor.setPosition(oldPosition);
  if (oldPosition > oldAnchor) {
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                        oldAnchor - oldPosition - numComments);
  } else {
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,
                        oldPosition - oldAnchor - numComments);
  }
  editor->setTextCursor(cursor);
}

void ProjectWindow::deploy() { ui->consoleText->clear(); }

void ProjectWindow::run(bool pressed) {
  //    QTextEdit *editor = static_cast<QTextEdit
  //    *>(ui->tabWidget->currentWidget());

  if (pressed) {
    if (build()) {
      //        ui->consoleText->clear();

      for (auto builder : m_builders) {
        builder->deploy();
      }
      for (auto builder : m_builders) {
        builder->run();
      }
      if (m_builders.size() == 0) {
        ui->actionRun->setChecked(false);
        printConsoleError(tr("Can't run. No builder available.").toStdString());
      }
    } else {
      programStopped();
    }
  } else {
    stop();
  }
}

void ProjectWindow::stop() {
  for (auto builder : m_builders) {
    builder->run(false);
  }
  // For some reason setChecked triggers the run action the wrong way... so need
  // to disbale these signals
  ui->actionRun->blockSignals(true);
  ui->actionRun->setChecked(false);
  ui->actionRun->blockSignals(false);
}

void ProjectWindow::tabChanged(int index) {
  Q_UNUSED(index)
  if (index >= 0) {
    CodeEditor *editor =
        static_cast<CodeEditor *>(ui->tabWidget->widget(index));
    m_highlighter->setDocument(editor->document());
    connect(editor, SIGNAL(requestAssistant(QPoint)), this,
            SLOT(showHelperMenu(QPoint)));
    updateCodeAnalysis(true);
  }
}

bool ProjectWindow::maybeSave() {
  QStringList modifiedFiles;
  QList<int> modifiedFilesTabs;
  for (int i = 0; i < ui->tabWidget->count(); i++) {
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
    if (editor->isChanged()) {
      QString name = editor->filename();
      if (name.isEmpty()) {
        name = "Untitled.stride";
      }
      modifiedFiles << name;
      modifiedFilesTabs << i;
    }
  }

  if (modifiedFiles.isEmpty()) {
    return true;
  }
  SaveChangedDialog d(this);
  d.setListContents(modifiedFiles);

  int but = d.exec();
  if (but == QDialog::Accepted) {
    foreach (int index, d.getSelected()) { saveFile(modifiedFilesTabs[index]); }
    return true;
  } else if (but == 100) { // Custom code for "don't save
    return true;
  } else {
    return false;
  }
}

void ProjectWindow::showDocumentation() {
  // TODO should all this be moved to the editor or related class?
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  QTextCursor cursor = editor->textCursor();
  if (cursor.selectedText() == "") {
    cursor.select(QTextCursor::WordUnderCursor);
  }
  QString word = cursor.selectedText();
  QString html = m_codeModel.getHtmlDocumentation(word);
  ui->documentationDockWidget->show();
  if (html.isEmpty()) {
    html = tr("Unknown type: %1").arg(word);
    ui->documentationWidget->setHtml(html);
  } else {
    ui->documentationWidget->setHtml(html);
    //        ui->documentationWidget->load(QUrl(html));
  }
}

void ProjectWindow::openGeneratedDir() {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  if (!editor->filename().isEmpty()) {
    QFileInfo info(editor->filename());
    QString dirName =
        info.absolutePath() + QDir::separator() + info.fileName() + "_Products";
    QDesktopServices::openUrl(QUrl("file://" + dirName));
  }
}

void ProjectWindow::cleanProject() {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  Q_ASSERT(!editor->filename().isEmpty());
  QFileInfo info(editor->filename());
  QString dirName =
      info.absolutePath() + QDir::separator() + info.fileName() + "_Products";
  if (QFile::exists(dirName)) {
    if (!QDir(dirName).removeRecursively()) {
      qDebug() << "Error cleaning project.";
    }
  }
}

void ProjectWindow::followSymbol() {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  QTextCursor cursor = editor->textCursor();
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
    QTextCursor cursor(
        editor->document()->findBlockByLineNumber(lineNumber - 1));
    editor->setTextCursor(cursor);
  }
}

void ProjectWindow::openRecent() {
  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
    loadFile(action->data().toString());
}

void ProjectWindow::showHelperMenu(QPoint where) {
  m_helperMenu.clear();

  //    bool platformChosen = false;
  //    foreach(AST *node, m_lastValidTree->getChildren()) {
  //        if (node->getNodeType() == AST::Platform) {
  //            platformChosen = true;
  //            break;
  //        }
  //    }
  //    QMenu *platformMenu = m_helperMenu.addMenu(tr("Platform"));
  //    QStringList platformList, platformCode;
  //    platformList << "Gamma" << "Arduino";
  //    platformCode << "use Gamma version 1.0 on PC" << "use Arduino
  //    version 1.0 on Uno"; for (int i = 0; i < platformList.size(); ++i) {
  //        QAction *newAction = platformMenu->addAction(platformList[i], this,
  //        SLOT(insertText())); newAction->setData(platformCode[i]);
  //    }
  QMenu *functionMenu = m_helperMenu.addMenu(tr("New function"));
  std::map<std::string, std::vector<ASTNode>> objs =
      m_codeModel.getSystem()->getImportTrees();
  for (auto namespaceGroup : objs) {
    for (auto obj : namespaceGroup.second) {
      if (obj->getNodeType() == AST::Declaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(obj.get());
        if (block->getObjectType() == "module") {
          QAction *newAction =
              functionMenu->addAction(QString::fromStdString(block->getName()),
                                      this, SLOT(insertText()));
          //                QString text =
          //                QString::fromStdString(block->getNamespace()); if
          //                (!text.isEmpty()) {
          //                    text += ".";
          //                }
          QString text = "";
          if (block->getScopeLevels()) {
            for (unsigned int i = 0; i < block->getScopeLevels(); i++) {
              text += QString::fromStdString(block->getScopeAt(i));
              text += "::";
            }
          }
          text += QString::fromStdString(block->getName()) + "(";
          ListNode *portList =
              static_cast<ListNode *>(block->getPropertyValue("ports").get());
          if (portList && portList->getNodeType() == AST::List) {
            for (ASTNode port : portList->getChildren()) {
              DeclarationNode *portBlock =
                  static_cast<DeclarationNode *>(port.get());
              ASTNode portName = portBlock->getPropertyValue("name");
              if (portName && portName->getNodeType() == AST::String) {
                std::string name =
                    static_cast<ValueNode *>(portName.get())->getStringValue();
                if (name.size() > 0) {
                  text += QString::fromStdString(name) + ":  ";
                }
              }
            }
          }
          text += ") ";
          newAction->setData(text);
        }
        //            node->deleteChildren();
        //            delete node;
      }
    }
  }

  m_helperMenu.exec(ui->tabWidget->currentWidget()->mapToGlobal(where));
}

void ProjectWindow::insertText(QString text) {
  if (text.isEmpty()) {
    text = static_cast<QAction *>(sender())->data().toString();
  }

  QTextEdit *editor = static_cast<QTextEdit *>(ui->tabWidget->currentWidget());
  editor->insertPlainText(text);
}

void ProjectWindow::find(QString query) {
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

void ProjectWindow::findNext() {
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

void ProjectWindow::findPrevious() {
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

void ProjectWindow::programStopped() { ui->actionRun->setChecked(false); }

void ProjectWindow::printConsoleText(std::string text) {
  if (text.size() != 0) {
    ui->consoleText->setTextColor(Qt::black);
    ui->consoleText->append(QString::fromStdString(text));
  }
}

void ProjectWindow::printConsoleError(std::string text) {
  if (text.size() != 0) {
    ui->consoleText->setTextColor(Qt::red);
    ui->consoleText->append(QString::fromStdString(text));
  }
}

void ProjectWindow::createResourceTreeItem(ASTNode inputNode,
                                           QTreeWidget *treeWidget) {

  //  CodeEditor *editor =
  //      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  if (inputNode->getNodeType() == AST::Declaration ||
      inputNode->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> declaration =
        std::static_pointer_cast<DeclarationNode>(inputNode);
    QStringList text;
    std::string frameworkName;
    auto frameworkNode = declaration->getCompilerProperty("framework");
    if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
      frameworkName =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    text << QString::fromStdString(declaration->getName() + " [" +
                                   frameworkName + "]");
    qDebug() << text << treeWidget;
    QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidget, text);
    newItem->setData(0, Qt::UserRole, text);
    treeWidget->addTopLevelItem(newItem);

    auto configsNode = declaration->getPropertyValue("configuration");
    if (configsNode) {
      for (auto config : configsNode->getChildren()) {
        text.clear();
        text << QString::fromStdString(ASTQuery::getNodeName(config));

        QTreeWidgetItem *newConfigItem = new QTreeWidgetItem(newItem, text);
        newItem->addChild(newConfigItem);
      }
    }
  } else {
    Q_ASSERT(0 == 1);
  }
}

QTreeWidgetItem *ProjectWindow::createTreeItem(ASTNode inputNode) {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  QTreeWidgetItem *newItem = nullptr;
  if (inputNode->getNodeType() == AST::Declaration ||
      inputNode->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> declaration =
        std::static_pointer_cast<DeclarationNode>(inputNode);
    QStringList itemText;
    QString type = QString::fromStdString(declaration->getObjectType());
    if (inputNode->getNodeType() == AST::BundleDeclaration) {
      std::shared_ptr<ListNode> indexList = declaration->getBundle()->index();
      if (indexList->getChildren().size() > 0) {
        ASTNode indexValue = indexList->getChildren()[0];
        if (indexValue->getNodeType() == AST::Int) {
          int size =
              std::static_pointer_cast<ValueNode>(indexValue)->getIntValue();
          type += QString("[%1]").arg(size);
        }
      } else {
        type += "[]";
      }
    }
    itemText << QString::fromStdString(declaration->getName()) << type;
    QVariantList fileInfo;
    fileInfo << QString::fromStdString(inputNode->getFilename());
    fileInfo << inputNode->getLine();
    if (!m_options["inspector.inspectorShowInternal"].toBool() &&
        (declaration->getName()[0] == '_' ||
         declaration->getObjectType()[0] == '_')) {
      return nullptr;
    }
    newItem = new QTreeWidgetItem(itemText);
    newItem->setData(0, Qt::UserRole, fileInfo);
    if (inputNode->getFilename() == "") {
      newItem->setBackground(0, Qt::green);
      newItem->setBackground(1, Qt::green);
    } else if (inputNode->getFilename() != editor->filename().toStdString()) {
      newItem->setBackground(0, Qt::lightGray);
      newItem->setBackground(1, Qt::lightGray);
    }
    for (auto property : declaration->getProperties()) {
      QStringList propertyItemText;
      propertyItemText << QString::fromStdString(property->getName());
      //                         <<
      //                         QString::fromStdString(decl->getObjectType());
      QTreeWidgetItem *propertyItem = new QTreeWidgetItem(propertyItemText);
      QVariantList fileInfo;
      fileInfo << QString::fromStdString(property->getFilename());
      fileInfo << property->getLine();
      propertyItem->setData(0, Qt::UserRole, fileInfo);
      if (property->getFilename() == "") {
        propertyItem->setBackground(0, Qt::green);
        propertyItem->setBackground(1, Qt::green);
      } else if (property->getFilename() != editor->filename().toStdString()) {
        propertyItem->setBackground(0, Qt::lightGray);
        propertyItem->setBackground(1, Qt::lightGray);
      }
      if (property->getValue()->getNodeType() == AST::List) {
        for (auto listMember : property->getValue()->getChildren()) {
          QTreeWidgetItem *subItem = createTreeItem(listMember);
          if (subItem) {
            QVariantList fileInfo;
            fileInfo << QString::fromStdString(listMember->getFilename());
            fileInfo << listMember->getLine();
            subItem->setData(0, Qt::UserRole, fileInfo);
            if (listMember->getFilename() == "") {
              subItem->setBackground(0, Qt::green);
              subItem->setBackground(1, Qt::green);
            } else if (listMember->getFilename() !=
                       editor->filename().toStdString()) {
              subItem->setBackground(0, Qt::lightGray);
              subItem->setBackground(1, Qt::lightGray);
            }
            propertyItem->addChild(subItem);
          }
        }

      } else {
        QTreeWidgetItem *subItem = createTreeItem(property->getValue());
        if (subItem) {
          QVariantList fileInfo;
          fileInfo << QString::fromStdString(
              property->getValue()->getFilename());
          fileInfo << property->getValue()->getLine();
          subItem->setData(0, Qt::UserRole, fileInfo);
          if (property->getValue()->getFilename() == "") {
            subItem->setBackground(0, Qt::green);
            subItem->setBackground(1, Qt::green);
          } else if (property->getValue()->getFilename() !=
                     editor->filename().toStdString()) {
            subItem->setBackground(0, Qt::lightGray);
            subItem->setBackground(1, Qt::lightGray);
          }
          propertyItem->addChild(subItem);
        }
      }
      newItem->addChild(propertyItem);
    }
  } /*else if (inputNode->getNodeType() == AST::String) {
    std::shared_ptr<ValueNode> stringValue =
        std::static_pointer_cast<ValueNode>(inputNode);
    QStringList itemText;
    itemText << "\"" + QString::fromStdString(stringValue->getStringValue() +
                                              "\"");
    newItem = new QTreeWidgetItem(itemText);
    QVariantList fileInfo;
    fileInfo << QString::fromStdString(stringValue->getFilename());
    fileInfo << stringValue->getLine();
    newItem->setData(0, Qt::UserRole, fileInfo);
  } else if (inputNode->getNodeType() == AST::Int) {
    std::shared_ptr<ValueNode> intNode =
        std::static_pointer_cast<ValueNode>(inputNode);
    QStringList itemText;
    itemText << QString::number(intNode->getIntValue());
    newItem = new QTreeWidgetItem(itemText);
    QVariantList fileInfo;
    fileInfo << QString::fromStdString(intNode->getFilename());
    fileInfo << intNode->getLine();
    newItem->setData(0, Qt::UserRole, fileInfo);
  } else if (inputNode->getNodeType() == AST::Real) {
    std::shared_ptr<ValueNode> realNode =
        std::static_pointer_cast<ValueNode>(inputNode);
    QStringList itemText;
    itemText << QString::number(realNode->getRealValue());
    newItem = new QTreeWidgetItem(itemText);
    QVariantList fileInfo;
    fileInfo << QString::fromStdString(realNode->getFilename());
    fileInfo << realNode->getLine();
    newItem->setData(0, Qt::UserRole, fileInfo);
  } else if (inputNode->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> blockNode =
        std::static_pointer_cast<BlockNode>(inputNode);
    QStringList itemText;
    itemText << QString::fromStdString(blockNode->getName());
    newItem = new QTreeWidgetItem(itemText);
    QVariantList fileInfo;
    fileInfo << QString::fromStdString(blockNode->getFilename());
    fileInfo << blockNode->getLine();
    newItem->setData(0, Qt::UserRole, fileInfo);
  }*/
  return newItem;
}

void ProjectWindow::prepareMenus() {

  QAction *recentFileAction = nullptr;
  for (auto i = 0; i < m_maxRecentFiles; ++i) {
    recentFileAction = new QAction(this);
    recentFileAction->setVisible(false);
    QObject::connect(recentFileAction, &QAction::triggered, this,
                     &ProjectWindow::openRecent);
    m_recentFilesActions.append(recentFileAction);
    ui->menuRecent_Files->addAction(recentFileAction);
  }
}

void ProjectWindow::setEditorText(QString code) {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  editor->setPlainText(code);
}

bool ProjectWindow::saveFile(int index) {
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
  if (!editor->isChanged()) {
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

bool ProjectWindow::saveFileAs(int index) {
  CodeEditor *editor;
  if (index == -1) {
    editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  } else {
    editor = static_cast<CodeEditor *>(ui->tabWidget->widget(index));
  }
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save file as:"));
  if (fileName.isEmpty()) {
    return false;
  }
  if (!fileName.endsWith(".stride")) {
    fileName.append(".stride");
  }
  editor->setFilename(fileName);
  editor->markChanged(true); // Needed as saveFile() exists without saving if
                             // file is not marked as modified.
  ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                            QFileInfo(fileName).fileName());

  ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),
                               QFileInfo(fileName).absoluteFilePath());
  return saveFile(index);
}

void ProjectWindow::closeTab(int index) {
  if (index < 0) {
    index = ui->tabWidget->currentIndex();
  }
  if (index >= ui->tabWidget->count()) {
    qDebug() << " ProjectWindow::closeTab(int index) invalid index " << index;
  }
  if (ui->tabWidget->count() > 1) { // TODO This should be set to ZERO
    CodeEditor *editor =
        static_cast<CodeEditor *>(ui->tabWidget->widget(index));
    if (editor->isChanged()) {
      QMessageBox::StandardButton result = QMessageBox::question(
          this, tr("File modified"), tr("File has been modified. Save?"),
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
      ui->tabWidget->removeTab(index);
      delete editor; // TODO better handling of this pointer (smarter pointer)
    }
  }
}

void ProjectWindow::loadFile() {
  QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("LoadFile"));
  if (!fileNames.isEmpty()) {
    foreach (QString fileName, fileNames) { loadFile(fileName); }
  }
}

void ProjectWindow::loadFile(QString fileName) {
  for (int i = 0; i < ui->tabWidget->count(); ++i) {
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
    if (editor->filename() == fileName) {
      ui->tabWidget->setCurrentWidget(editor);
      return;
    }
  }
  QFile codeFile(fileName);
  if (!codeFile.open(QIODevice::ReadOnly)) { // ReadWrite creates the file if it
                                             // doesn't exist
    qDebug() << "Error opening code file!";
    return;
  }
  newFile();
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  setEditorText(codeFile.readAll());
  editor->setFilename(QFileInfo(fileName).absoluteFilePath());
  updateCodeAnalysis();
  qApp->processEvents();
  editor->markChanged(false);
  ui->tabWidget->setTabText(ui->tabWidget->currentIndex(),
                            QFileInfo(fileName).fileName());
  ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(),
                               QFileInfo(fileName).absoluteFilePath());
  codeFile.close();
  markModified();

  adjustForCurrentFile(fileName);
}

void ProjectWindow::openOptionsDialog() {
  // First set current values
  ConfigDialog config(this);
  QFont font = QFont(m_options["editor.fontFamily"].toString(),
                     m_options["editor.fontSize"].toInt(),
                     m_options["editor.fontWeight"].toInt(),
                     m_options["editor.fontItalic"].toBool());
  font.setPointSizeF(
      m_options["editor.fontSize"].toDouble()); // Font constructor takes int
  config.setFont(font);

  config.setAutoComplete(m_options["editor.autoComplete"].toBool());
  config.setInspectorShowAll(
      m_options["inspector.inspectorShowInternal"].toBool());

  QMap<QString, QTextCharFormat> formats = m_highlighter->formats();
  config.setHighlighterFormats(formats);

  // Connect
  connect(&config, SIGNAL(requestHighlighterPreset(int)), m_highlighter,
          SLOT(setFormatPreset(int)));
  connect(m_highlighter,
          SIGNAL(currentHighlightingChanged(QMap<QString, QTextCharFormat> &)),
          &config,
          SLOT(setHighlighterFormats(QMap<QString, QTextCharFormat> &)));

  int result = config.exec();
  // Get values back
  if (result == QDialog::Accepted) {
    writeSettings();
    QFont font = config.font();
    m_options["editor.fontFamily"] = font.family();
    m_options["editor.fontSize"] = font.pointSizeF();
    m_options["editor.fontWeight"] = font.weight();
    m_options["editor.fontItalic"] = font.italic();
    m_options["editor.autoComplete"] = config.autoComplete();
    m_options["inspector.inspectorShowInternal"] = config.inspectorShowAll();
    m_highlighter->setFormats(config.highlighterFormats());

    updateEditorSettings();
    fillInspectorTree();
  }
}

void ProjectWindow::openManageStriderootDialog() {
  StriderootManagementDialog dialog;
  dialog.m_strideRoot = m_environment["striderootPath"].toString();

  dialog.prepare();
  int result = dialog.exec();

  if (result == QDialog::Accepted) {
    m_environment["striderootPath"] = dialog.m_strideRoot;
    writeSettings();
  }
}

void ProjectWindow::openManageLocalDialog() {
  LocalManagementDialog dialog(m_environment["striderootPath"].toString());

  /*int result = */ dialog.exec();
}

void ProjectWindow::updateCodeAnalysis(bool force) {
  m_codeModelTimer.stop();
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  if ((QApplication::activeWindow() == this && editor->changedSinceParse()) ||
      m_startingUp || force) {
    editor->markParsed();
    m_codeModel.updateCodeAnalysis(editor->document()->toPlainText(),
                                   m_environment["striderootPath"].toString(),
                                   editor->filename());
    m_highlighter->setBlockTypes(m_codeModel.getTypes());
    m_highlighter->setFunctions(m_codeModel.getFunctions());
    m_highlighter->setBuiltinObjects(m_codeModel.getObjectNames());
    editor->setErrors(m_codeModel.getErrors());
    fillInspectorTree();
  }
  QPoint position = editor->mapFromGlobal(QCursor::pos());
  position.rx() -= editor->lineNumberAreaWidth();
  QTextCursor cursor = editor->cursorForPosition(position);
  cursor.select(QTextCursor::WordUnderCursor);
  //    qDebug() << cursor.selectedText();
  editor->setToolTipText(m_codeModel.getTooltipText(cursor.selectedText()));

  m_codeModelTimer.start();
}

void ProjectWindow::connectActions() {

  connect(ui->actionNew_Project, SIGNAL(triggered()), this, SLOT(newFile()));
  connect(ui->actionConfigure_System, SIGNAL(triggered()), this,
          SLOT(configureSystem()));
  connect(ui->actionBuild, SIGNAL(triggered()), this, SLOT(build()));
  connect(ui->actionComment, SIGNAL(triggered(bool)), this,
          SLOT(commentSection()));
  connect(ui->actionUpload, SIGNAL(triggered()), this, SLOT(deploy()));
  connect(ui->actionRun, SIGNAL(toggled(bool)), this, SLOT(run(bool)));
  connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
  connect(ui->actionSave_as, SIGNAL(triggered()), this, SLOT(saveFileAs()));
  connect(ui->actionClose_Tab, SIGNAL(triggered()), this, SLOT(closeTab()));
  connect(ui->actionOptions, SIGNAL(triggered()), this,
          SLOT(openOptionsDialog()));
  connect(ui->actionManage_Strideroot, SIGNAL(triggered()), this,
          SLOT(openManageStriderootDialog()));
  connect(ui->actionManage_Local, SIGNAL(triggered()), this,
          SLOT(openManageLocalDialog()));
  connect(ui->actionLoad_File, SIGNAL(triggered()), this, SLOT(loadFile()));
  connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stop()));
  connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionShow_Documentation, SIGNAL(triggered()), this,
          SLOT(showDocumentation()));
  connect(ui->actionFollow_Symbol, SIGNAL(triggered()), this,
          SLOT(followSymbol()));
  connect(ui->actionOpen_Generated_Code, SIGNAL(triggered()), this,
          SLOT(openGeneratedDir()));
  connect(ui->actionClean_Project, SIGNAL(triggered()), this,
          SLOT(cleanProject()));

  connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(find()));
  connect(ui->actionFind_Next, SIGNAL(triggered()), this, SLOT(findNext()));
  connect(ui->actionFind_Previous, SIGNAL(triggered()), this,
          SLOT(findPrevious()));

  connect(m_searchWidget.data()->getFindNextButton(), SIGNAL(released()), this,
          SLOT(findNext()));
  connect(m_searchWidget.data()->getFindPreviousButton(), SIGNAL(released()),
          this, SLOT(findPrevious()));

  //    connect(m_project, SIGNAL(outputText(QString)), this,
  //    SLOT(printConsoleText(QString))); connect(m_project,
  //    SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
  //    connect(m_project, SIGNAL(programStopped()), this,
  //    SLOT(programStopped()));
}

void ProjectWindow::connectShortcuts() {
  ui->actionNew_Project->setShortcut(QKeySequence("Ctrl+N"));
  ui->actionBuild->setShortcut(QKeySequence("Ctrl+B"));
  ui->actionComment->setShortcut(QKeySequence("Ctrl+/"));
  ui->actionUpload->setShortcut(QKeySequence("Ctrl+U"));

  ui->actionRun->setShortcut(QKeySequence("Ctrl+R"));
  ui->actionRefresh->setShortcut(QKeySequence(""));
  ui->actionSave->setShortcut(QKeySequence("Ctrl+S"));
  ui->actionOptions->setShortcut(QKeySequence(""));
  ui->actionLoad_File->setShortcut(QKeySequence("Ctrl+O"));
  ui->actionStop->setShortcut(QKeySequence("Ctrl+Space"));
  ui->actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
}

void ProjectWindow::readSettings(bool resetOpenFiles) {
  QSettings settings("Stride", "StrideIDE", this);
  settings.beginGroup("project");
  m_options["editor.fontFamily"] =
      settings.value("editor.fontFamily", "Courier").toString();
  m_options["editor.fontSize"] =
      settings.value("editor.fontSize", 10.0).toDouble();
  m_options["editor.fontWeight"] =
      settings.value("editor.fontSize", QFont::Normal).toInt();
  m_options["editor.fontItalic"] =
      settings.value("editor.fontItalic", false).toBool();
  m_options["editor.autoComplete"] =
      settings.value("editor.autoComplete", true).toBool();
  settings.endGroup();
  updateEditorSettings();

  settings.beginGroup("highlighter");
  QMap<QString, QTextCharFormat> formats = m_highlighter->formats();
  QStringList keys = settings.childGroups();
  foreach (QString key, keys) {
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

  settings.beginGroup("environment");
  m_environment["striderootPath"] =
      settings.value("striderootPath", "../../Stride/strideroot").toString();
  settings.endGroup();

  settings.beginGroup("GUI");
  if (settings.contains("geometry")) {
    this->restoreGeometry(settings.value("geometry").toByteArray());
  }
  if (settings.contains("windowState")) {
    if (!this->restoreState(settings.value("windowState").toByteArray())) {
      qDebug() << "Error recalling window state. (Version mismatch?)";
    }
  }
  int size = settings.beginReadArray("openDocuments");
  QStringList filesToOpen;
  for (int i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    filesToOpen << settings.value("fileName").toString();
  }
  settings.endArray();

  if (!resetOpenFiles) {
    QStringList missingFiles;
    foreach (QString fileName, filesToOpen) {
      if (fileName.isEmpty()) {
        newFile();
      } else {
        if (QFile::exists(fileName)) {
          loadFile(fileName);
        } else {
          missingFiles << fileName;
        }
      }
    }

    if (missingFiles.size() > 0) {
      QMessageBox::warning(this, tr("File not found"),
                           tr("Previously open file(s) %1 not found.")
                               .arg(missingFiles.join(",")));
    }
    int lastIndex = settings.value("lastIndex", -1)
                        .toInt(); // Used later after files are loaded
    ui->tabWidget->setCurrentIndex(lastIndex);
  }

  m_options["inspector.inspectorShowInternal"] =
      settings.value("inspectorShowInternal", true).toBool();
  settings.endGroup();
}

void ProjectWindow::writeSettings() {
  QSettings settings("Stride", "StrideIDE", this);
  settings.beginGroup("project");
  QStringList keys = m_options.keys();
  for (QString key : keys) {
    settings.setValue(key, m_options[key]);
  }
  settings.endGroup();

  settings.beginGroup("highlighter");
  QMapIterator<QString, QTextCharFormat> i(m_highlighter->formats());
  while (i.hasNext()) {
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

  settings.beginGroup("environment");
  settings.setValue("striderootPath", m_environment["striderootPath"]);
  settings.endGroup();

  settings.beginGroup("GUI");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.setValue("lastIndex", ui->tabWidget->currentIndex());
  settings.beginWriteArray("openDocuments");
  for (int i = 0; i < ui->tabWidget->count(); i++) {
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
    settings.setArrayIndex(i);
    settings.setValue("fileName", editor->filename());
  }
  settings.endArray();
  settings.setValue("inspectorShowInternal",
                    m_options["inspector.inspectorShowInternal"].toBool());
  settings.endGroup();
}

void ProjectWindow::updateEditorSettings() {
  for (int i = 0; i < ui->tabWidget->count(); i++) {
    QString fontFamily = m_options["editor.fontFamily"].toString();
    int fontSize = m_options["editor.fontSize"].toInt();
    int fontWeight = m_options["editor.fontWeight"].toInt();
    bool fontItalic = m_options["editor.fontItalic"].toBool();
    QFont font = QFont(fontFamily, fontSize, fontWeight, fontItalic);
    font.setPointSizeF(fontSize); // Font constructor takes int
    CodeEditor *editor = static_cast<CodeEditor *>(ui->tabWidget->widget(i));
    editor->setFont(font);

    //    const int tabStop = 4; // 4 characters
    //    QFontMetrics metrics(font);
    //    editor->setTabStopWidth(tabStop * metrics.width(' '));

    editor->setAutoComplete(m_options["editor.autoComplete"].toBool());
  }
}

void ProjectWindow::adjustForCurrentFile(const QString &filePath) {
  //      currentFilePath = filePath;
  //      setWindowFilePath(currentFilePath);

  QSettings settings;
  QStringList recentFilePaths = settings.value("recentFiles").toStringList();
  recentFilePaths.removeAll(filePath);
  recentFilePaths.prepend(filePath);
  while (recentFilePaths.size() > m_maxRecentFiles)
    recentFilePaths.removeLast();
  settings.setValue("recentFiles", recentFilePaths);

  // see note
  updateRecentActionList();
}

void ProjectWindow::updateRecentActionList() {
  QSettings settings;
  QStringList recentFilePaths = settings.value("recentFiles").toStringList();

  int itEnd = 0;
  if (recentFilePaths.size() <= m_maxRecentFiles)
    itEnd = recentFilePaths.size();
  else
    itEnd = m_maxRecentFiles;

  for (auto i = 0; i < itEnd; ++i) {
    QString strippedName = QFileInfo(recentFilePaths.at(i)).fileName();
    m_recentFilesActions.at(i)->setText(strippedName);
    m_recentFilesActions.at(i)->setData(recentFilePaths.at(i));
    m_recentFilesActions.at(i)->setVisible(true);
  }

  for (auto i = itEnd; i < m_maxRecentFiles; ++i)
    m_recentFilesActions.at(i)->setVisible(false);
}

void ProjectWindow::fillInspectorTree() {
  // Fill inspector tree
  auto tree = m_codeModel.getOptimizedTree();
  // TODO keep open leafs open
  ui->treeWidget->clear();
  ui->treeWidget->setColumnCount(2);
  ui->treeWidget->setHeaderLabels(QStringList() << "Name"
                                                << "Type");

  ui->treeWidget->header()->setSectionResizeMode(0,
                                                 QHeaderView::ResizeToContents);
  ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  ui->treeWidget->setSortingEnabled(false);

  ui->resourcesTreeWidget->clear();
  ui->resourcesTreeWidget->setColumnCount(1);

  //  ui->treeWidget->setHeaderLabels(QStringList() << "Name");

  //  ui->resourcesTreeWidget->header()->setSectionResizeMode(
  //      0, QHeaderView::ResizeToContents);
  //  ui->resourcesTreeWidget->header()->setSectionResizeMode(1,
  //                                                          QHeaderView::Stretch);
  //  ui->resourcesTreeWidget->setSortingEnabled(false);

  if (tree) {
    for (auto &node : tree->getChildren()) {
      if (node->getNodeType() == AST::Declaration ||
          node->getNodeType() == AST::BundleDeclaration) {
        std::shared_ptr<DeclarationNode> decl =
            std::static_pointer_cast<DeclarationNode>(node);

        if (decl->getObjectType() == "resource" ||
            decl->getObjectType() == "domainResource") {
          createResourceTreeItem(decl, ui->resourcesTreeWidget);
        } else {
          QTreeWidgetItem *newItem = createTreeItem(decl);
          if (newItem) {

            QString tooltipText;
            auto writes = decl->getCompilerProperty("writes");
            if (writes) {
              tooltipText += "Writes:\n";
              for (auto &write : writes->getChildren()) {
                if (write->getNodeType() == AST::String) {
                  tooltipText += QString::fromStdString(
                      std::static_pointer_cast<ValueNode>(write)
                          ->getStringValue());
                } else {
                  tooltipText +=
                      QString::fromStdString(ASTQuery::getNodeName(write));
                }
                tooltipText += "\n";
              }
            }
            auto reads = decl->getCompilerProperty("reads");
            if (reads) {
              tooltipText += "Reads:\n";
              for (auto &read : reads->getChildren()) {
                if (read->getNodeType() == AST::String) {
                  tooltipText += QString::fromStdString(
                      std::static_pointer_cast<ValueNode>(read)
                          ->getStringValue());
                } else {
                  tooltipText +=
                      QString::fromStdString(ASTQuery::getNodeName(read));
                }
                tooltipText += "\n";
              }
            }
            newItem->setToolTip(0, tooltipText);

            ui->treeWidget->addTopLevelItem(newItem);
          }
        }
      } else if (node->getNodeType() == AST::Stream) {
        // Should we display streams too?
      }
    }
    ui->treeWidget->setSortingEnabled(true);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);

    ui->resourcesTreeWidget->setSortingEnabled(true);
    ui->resourcesTreeWidget->sortByColumn(0, Qt::AscendingOrder);
  }
}

void ProjectWindow::newFile() {
  // Create editor tab
  CodeEditor *editor = new CodeEditor(this, &m_codeModel);
  editor->setFilename("");

  int index = ui->tabWidget->insertTab(ui->tabWidget->currentIndex() + 1,
                                       editor, "untitled");
  ui->tabWidget->setCurrentIndex(index);
  updateEditorSettings();
  m_highlighter->setDocument(editor->document());
  QObject::connect(editor, SIGNAL(textChanged()), this, SLOT(markModified()));
  QObject::connect(editor, SIGNAL(textChanged()), this, SLOT(resetCodeTimer()));
  QObject::connect(editor, SIGNAL(requestAssistant(QPoint)), this,
                   SLOT(showHelperMenu(QPoint)));
  editor->setFocus();
}

void ProjectWindow::markModified() {
  int currentIndex = ui->tabWidget->currentIndex();
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  QColor textColor;
  if (editor->isChanged()) {
    textColor = Qt::red;
  } else {
    QPalette p = this->palette();
    textColor = p.color(QPalette::WindowText);
  }
  ui->tabWidget->tabBar()->setTabTextColor(currentIndex, textColor);
}

void ProjectWindow::configureSystem() {
  CodeEditor *editor =
      static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
  std::vector<ASTNode> optionTrees;
  ASTNode tree =
      ASTFunctions::parseFile(editor->filename().toStdString().c_str());
  if (tree) {
    m_codeModel.updateCodeAnalysis(editor->document()->toPlainText(),
                                   m_environment["striderootPath"].toString(),
                                   editor->filename());
  } else {
    QMessageBox::warning(this, tr("No system defined"),
                         tr("Can't configure system. No system defined"));
    return;
  }

  if (!m_codeModel.getOptimizedTree() || !m_codeModel.getSystem()) {
    QMessageBox::critical(
        this, tr("Error"),
        "Error parsing or processing system. Can't configure.",
        QMessageBox::Ok);
    return;
  }
  auto children = m_codeModel.getOptimizedTree()->getChildren();
  tree->setChildren(children);
  for (auto system : ASTQuery::getSystemNodes(tree)) {
    optionTrees = StrideSystem::getOptionTrees(
        m_environment["striderootPath"].toString().toStdString() + "/systems/" +
        system->platformName() + "/" + std::to_string(system->majorVersion()) +
        "." + std::to_string(system->minorVersion()));
    break; // Just use the first system declaration.
  }

  // Read configuration from file
  SystemConfiguration systemConfig;
  systemConfig.readConfiguration(editor->filename().toStdString());

  QDialog optionsDialog(this);
  QTabWidget *mainLayout = new QTabWidget;
  QMap<QString, QSpinBox *> spinBoxes;
  QMap<QString, QComboBox *> comboBoxes;
  QMap<QString, QLineEdit *> lineEdits;
  QMap<QString, QComboBox *> intComboBoxes;
  QMap<QString, QSpinBox *> overrideSpinBoxes;
  QMap<QString, QComboBox *> overrideComboBoxes;
  QMap<QString, QComboBox *> overrideIntComboBoxes;

  QMap<QString, QSpinBox *> resourceSpinBoxes;
  QMap<QString, QComboBox *> resourceComboBoxes;
  QMap<QString, QLineEdit *> resourceLineEdits;
  QMap<QString, QComboBox *> resourceIntComboBoxes;

  for (auto node : children) {
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "resource" ||
          decl->getObjectType() == "domainResource") {
        continue;
        auto configuration = decl->getPropertyValue("configuration");
        if (configuration->getChildren().size() == 0) {
          continue;
        }
        QWidget *resourceGroupBox = new QWidget(&optionsDialog);
        QVBoxLayout *resourceGroupLayout = new QVBoxLayout;
        resourceGroupBox->setLayout(resourceGroupLayout);
        std::string groupName;
        auto resourceName = decl->getPropertyValue("name");
        if (resourceName && resourceName->getNodeType() == AST::String) {
          groupName = std::static_pointer_cast<ValueNode>(resourceName)
                          ->getStringValue();
        } else {
          groupName = decl->getName();
        }
        if (configuration && configuration->getNodeType() == AST::List) {

          QWidget *optionWidget = new QWidget(resourceGroupBox);
          QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);

          optionWidget->setLayout(optionLayout);
          resourceGroupLayout->addWidget(optionWidget);
          mainLayout->addTab(resourceGroupBox,
                             QString::fromStdString(groupName));

          for (auto config : configuration->getChildren()) {
            if (config->getNodeType() == AST::Block) {
              auto configDecl = ASTQuery::findDeclarationByName(
                  std::static_pointer_cast<BlockNode>(config)->getName(), {},
                  tree);
              if (configDecl) {
                QString optionName;
                auto configName = configDecl->getPropertyValue("name");
                if (configName && configName->getNodeType() == AST::String) {
                  optionName = QString::fromStdString(
                      std::static_pointer_cast<ValueNode>(configName)
                          ->getStringValue());
                } else {
                  optionName = QString::fromStdString(configDecl->getName());
                }
                bool showValues = false;
                auto possibleValues =
                    configDecl->getPropertyValue("possibleValues");
                if (possibleValues &&
                    possibleValues->getNodeType() == AST::List &&
                    possibleValues->getChildren().size() > 0) {
                  showValues = true;
                }
                auto type = configDecl->getPropertyValue("type");
                if ((!type) || !(type->getNodeType() == AST::Block)) {
                  qDebug()
                      << "ERROR: no type provided by resourceConfiguration";
                  continue;
                }
                auto typeBlock = std::static_pointer_cast<BlockNode>(type);
                if (typeBlock->getName() == "_IntLiteral") {

                  if (!showValues) {

                    optionLayout->addWidget(new QLabel(optionName));
                    QSpinBox *spinBox = new QSpinBox(optionWidget);

                    bool configFound = false;
                    for (auto frameworkConfig :
                         systemConfig.resourceConfigurations) {
                      for (auto resourceConfig : frameworkConfig.second) {
                        if (resourceConfig->getName() ==
                            configDecl->getName()) {
                          auto valueProp =
                              resourceConfig->getPropertyValue("value");
                          if (valueProp &&
                              valueProp->getNodeType() == AST::Int) {
                            spinBox->setValue(
                                std::static_pointer_cast<ValueNode>(valueProp)
                                    ->getIntValue());
                            configFound = true;
                          } else {
                            std::cerr
                                << "Found configuration for "
                                << resourceConfig->getName()
                                << " but value type mismatch. Using default."
                                << std::endl;
                          }

                          ASTNode maxNode =
                              configDecl->getPropertyValue("maximum");
                          ASTNode minNode =
                              configDecl->getPropertyValue("minimum");

                          if (minNode && minNode->getNodeType() == AST::Int) {
                            spinBox->setMinimum(
                                std::static_pointer_cast<ValueNode>(minNode)
                                    ->getIntValue());
                          }
                          if (maxNode && maxNode->getNodeType() == AST::Int) {
                            spinBox->setMaximum(
                                std::static_pointer_cast<ValueNode>(maxNode)
                                    ->getIntValue());
                          }

                          break;
                        }
                      }
                    }
                    if (!configFound) { // Use default
                      ASTNode defaultNode =
                          configDecl->getPropertyValue("default");
                      if (defaultNode &&
                          defaultNode->getNodeType() == AST::Int) {
                        spinBox->setValue(
                            std::static_pointer_cast<ValueNode>(defaultNode)
                                ->getIntValue());
                      }
                    }

                    optionLayout->addWidget(spinBox);
                    resourceSpinBoxes[QString::fromStdString(
                        configDecl->getName())] = spinBox;
                  } else { // Show options

                    optionLayout->addWidget(new QLabel(optionName));
                    QComboBox *widget = new QComboBox(optionWidget);
                    std::shared_ptr<ListNode> list =
                        std::static_pointer_cast<ListNode>(possibleValues);
                    for (ASTNode member : list->getChildren()) {
                      if (member && member->getNodeType() == AST::Int) {
                        widget->addItem(QString::number(
                            std::static_pointer_cast<ValueNode>(member)
                                ->getIntValue()));
                      }
                    }
                    bool configFound = false;
                    for (auto frameworkConfig :
                         systemConfig.resourceConfigurations) {
                      for (auto resourceConfig : frameworkConfig.second) {
                        if (resourceConfig->getName() ==
                            configDecl->getName()) {
                          auto valueProp =
                              resourceConfig->getPropertyValue("value");
                          if (valueProp &&
                              valueProp->getNodeType() == AST::Int) {
                            widget->setCurrentText(QString::number(
                                std::static_pointer_cast<ValueNode>(valueProp)
                                    ->getIntValue()));
                            configFound = true;
                          } else {
                            std::cerr
                                << "Found configuration for "
                                << resourceConfig->getName()
                                << " but value type mismatch. Using default."
                                << std::endl;
                          }

                          break;
                        }
                      }
                    }
                    if (!configFound) { // Use default
                      ASTNode defaultNode =
                          configDecl->getPropertyValue("default");

                      if (defaultNode &&
                          defaultNode->getNodeType() == AST::Int) {

                        widget->setCurrentText(QString::number(
                            std::static_pointer_cast<ValueNode>(defaultNode)
                                ->getIntValue()));
                      }
                    }

                    optionLayout->addWidget(widget);
                    resourceIntComboBoxes[QString::fromStdString(
                        configDecl->getName())] = widget;
                  }

                } else if (typeBlock->getName() == "_StringLiteral") {

                  if (!showValues) {

                    optionLayout->addWidget(new QLabel(optionName));

                    QLineEdit *widget = new QLineEdit(optionWidget);

                    bool configFound = false;
                    for (auto frameworkConfig :
                         systemConfig.resourceConfigurations) {
                      for (auto resourceConfig : frameworkConfig.second) {
                        if (resourceConfig->getName() ==
                            configDecl->getName()) {
                          auto valueProp =
                              resourceConfig->getPropertyValue("value");
                          if (valueProp &&
                              valueProp->getNodeType() == AST::String) {
                            widget->setText(QString::fromStdString(
                                std::static_pointer_cast<ValueNode>(valueProp)
                                    ->getStringValue()));
                            configFound = true;
                          } else {
                            std::cerr
                                << "Found configuration for "
                                << resourceConfig->getName()
                                << " but value type mismatch. Using default."
                                << std::endl;
                          }
                          break;
                        }
                      }
                    }
                    if (!configFound) { // Use default
                      ASTNode defaultNode =
                          configDecl->getPropertyValue("default");
                      if (defaultNode &&
                          defaultNode->getNodeType() == AST::String) {
                        widget->setText(QString::fromStdString(
                            std::static_pointer_cast<ValueNode>(defaultNode)
                                ->getStringValue()));
                      }
                    }

                    optionLayout->addWidget(widget);
                    resourceLineEdits[QString::fromStdString(
                        configDecl->getName())] = widget;
                  } else { // Show options

                    optionLayout->addWidget(new QLabel(optionName));
                    QComboBox *widget = new QComboBox(optionWidget);
                    std::shared_ptr<ListNode> list =
                        std::static_pointer_cast<ListNode>(possibleValues);
                    for (ASTNode member : list->getChildren()) {
                      if (member && member->getNodeType() == AST::String) {
                        widget->addItem(QString::fromStdString(
                            std::static_pointer_cast<ValueNode>(member)
                                ->getStringValue()));
                      }
                    }
                    bool configFound = false;
                    for (auto frameworkConfig :
                         systemConfig.resourceConfigurations) {
                      for (auto resourceConfig : frameworkConfig.second) {
                        if (resourceConfig->getName() ==
                            configDecl->getName()) {
                          auto valueProp =
                              resourceConfig->getPropertyValue("value");
                          if (valueProp &&
                              valueProp->getNodeType() == AST::String) {
                            widget->setCurrentText(QString::fromStdString(
                                std::static_pointer_cast<ValueNode>(valueProp)
                                    ->getStringValue()));
                            configFound = true;
                          } else {
                            std::cerr
                                << "Found configuration for "
                                << resourceConfig->getName()
                                << " but value type mismatch. Using default."
                                << std::endl;
                          }

                          break;
                        }
                      }
                    }
                    if (!configFound) { // Use default
                      ASTNode defaultNode =
                          configDecl->getPropertyValue("default");

                      if (defaultNode &&
                          defaultNode->getNodeType() == AST::Int) {

                        widget->setCurrentText(QString::number(
                            std::static_pointer_cast<ValueNode>(defaultNode)
                                ->getIntValue()));
                      }
                    }

                    optionLayout->addWidget(widget);
                    resourceComboBoxes[QString::fromStdString(
                        configDecl->getName())] = widget;
                  }
                } else {
                  qDebug() << "ERROR: unsupported config type";
                }
              }
            } else {

              qDebug() << "Unexpected type in resource configuration "
                       << QString::fromStdString(config->getFilename())
                       << QString::fromStdString(
                              std::to_string(config->getLine()));
            }
          }
        }
      }
    }
  }

  for (ASTNode optionTree : optionTrees) {
    QWidget *groupBox = new QWidget(&optionsDialog);

    QVBoxLayout *groupLayout = new QVBoxLayout;
    std::string groupName;

    for (ASTNode option : optionTree->getChildren()) {
      QWidget *optionWidget = new QWidget(groupBox);
      QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);
      if (option->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> optionDecl =
            std::static_pointer_cast<DeclarationNode>(option);
        std::string type = optionDecl->getObjectType();
        if (type == "optionGroup") {
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            groupName =
                std::static_pointer_cast<ValueNode>(nameNode)->getStringValue();
          }
        } else if (type == "intOption") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          QStringList buildPlatforms;
          ASTNode buildPlatformsNode =
              optionDecl->getPropertyValue("buildPlatforms");
          if (buildPlatformsNode &&
              buildPlatformsNode->getNodeType() == AST::List) {
            for (ASTNode member : buildPlatformsNode->getChildren()) {
              if (member->getNodeType() == AST::String) {
                buildPlatforms << QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue());
              }
            }
          }
          optionLayout->addWidget(new QLabel(optionName));

          ASTNode maxNode = optionDecl->getPropertyValue("maximum");
          ASTNode minNode = optionDecl->getPropertyValue("minimum");
          QSpinBox *spinBox = new QSpinBox(optionWidget);

          if (minNode && minNode->getNodeType() == AST::Int) {
            spinBox->setMinimum(
                std::static_pointer_cast<ValueNode>(minNode)->getIntValue());
          }
          if (maxNode && maxNode->getNodeType() == AST::Int) {
            spinBox->setMaximum(
                std::static_pointer_cast<ValueNode>(maxNode)->getIntValue());
          }

          if (systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
              systemConfig.frameworkConfigurations["all"].end()) {
            spinBox->setValue(std::get<int64_t>(
                systemConfig
                    .frameworkConfigurations["all"][optionDecl->getName()]));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::Int) {
              spinBox->setValue(std::static_pointer_cast<ValueNode>(defaultNode)
                                    ->getIntValue());
            }
          }
          optionLayout->addWidget(spinBox);

          ASTNode metaNode = optionDecl->getPropertyValue("meta");
          if (metaNode && metaNode->getNodeType() == AST::String) {
            optionWidget->setToolTip(QString::fromStdString(
                std::static_pointer_cast<ValueNode>(metaNode)
                    ->getStringValue()));
          }
          spinBoxes[QString::fromStdString(optionDecl->getName())] = spinBox;
        } else if (type == "stringOption") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          QStringList buildPlatforms;
          ASTNode buildPlatformsNode =
              optionDecl->getPropertyValue("buildPlatforms");
          if (buildPlatformsNode &&
              buildPlatformsNode->getNodeType() == AST::List) {
            for (ASTNode member : buildPlatformsNode->getChildren()) {
              if (member->getNodeType() == AST::String) {
                buildPlatforms << QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue());
              }
            }
          }
          optionLayout->addWidget(new QLabel(optionName));

          ASTNode maxNode = optionDecl->getPropertyValue("maximum");
          ASTNode minNode = optionDecl->getPropertyValue("minimum");
          QLineEdit *spinBox = new QLineEdit(optionWidget);

          //                    if (minNode && minNode->getNodeType() ==
          //                    AST::Int) {
          //                        spinBox->setMinimum(static_pointer_cast<ValueNode>(minNode)->getIntValue());
          //                    }
          //                    if (maxNode && maxNode->getNodeType() ==
          //                    AST::Int) {
          //                        spinBox->setMaximum(static_pointer_cast<ValueNode>(maxNode)->getIntValue());
          //                    }

          if (systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
              systemConfig.frameworkConfigurations["all"].end()) {
            spinBox->setText(QString::fromStdString(std::get<std::string>(
                systemConfig
                    .frameworkConfigurations["all"][optionDecl->getName()])));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::String) {
              spinBox->setText(QString::fromStdString(
                  std::static_pointer_cast<ValueNode>(defaultNode)
                      ->getStringValue()));
            }
          }
          optionLayout->addWidget(spinBox);

          ASTNode metaNode = optionDecl->getPropertyValue("meta");
          if (metaNode && metaNode->getNodeType() == AST::String) {
            optionWidget->setToolTip(QString::fromStdString(
                std::static_pointer_cast<ValueNode>(metaNode)
                    ->getStringValue()));
          }
          lineEdits[QString::fromStdString(optionDecl->getName())] = spinBox;
        } else if (type == "fileOption") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          QStringList buildPlatforms;
          ASTNode buildPlatformsNode =
              optionDecl->getPropertyValue("buildPlatforms");
          if (buildPlatformsNode &&
              buildPlatformsNode->getNodeType() == AST::List) {
            for (ASTNode member : buildPlatformsNode->getChildren()) {
              if (member->getNodeType() == AST::String) {
                buildPlatforms << QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue());
              }
            }
          }
          optionLayout->addWidget(new QLabel(optionName));

          QLineEdit *spinBox = new QLineEdit(optionWidget);
          QPushButton *browseButton = new QPushButton("...", optionWidget);
          QObject::connect(browseButton, &QPushButton::clicked, this,
                           [&, spinBox]() {
                             QString fileName = QFileDialog::getOpenFileName(
                                 this, tr("Find Files"), QDir::currentPath());
                             qDebug() << fileName;
                             if (fileName.size() > 0) {
                               spinBox->setText(fileName);
                             }
                           });

          if (systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
              systemConfig.frameworkConfigurations["all"].end()) {
            spinBox->setText(QString::fromStdString(std::get<std::string>(
                systemConfig
                    .frameworkConfigurations["all"][optionDecl->getName()])));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::String) {
              spinBox->setText(QString::fromStdString(
                  std::static_pointer_cast<ValueNode>(defaultNode)
                      ->getStringValue()));
            }
          }
          optionLayout->addWidget(spinBox);
          optionLayout->addWidget(browseButton);

          ASTNode metaNode = optionDecl->getPropertyValue("meta");
          if (metaNode && metaNode->getNodeType() == AST::String) {
            optionWidget->setToolTip(QString::fromStdString(
                std::static_pointer_cast<ValueNode>(metaNode)
                    ->getStringValue()));
          }
          lineEdits[QString::fromStdString(optionDecl->getName())] = spinBox;
        } else if (type == "pathOption") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          QStringList buildPlatforms;
          ASTNode buildPlatformsNode =
              optionDecl->getPropertyValue("buildPlatforms");
          if (buildPlatformsNode &&
              buildPlatformsNode->getNodeType() == AST::List) {
            for (ASTNode member : buildPlatformsNode->getChildren()) {
              if (member->getNodeType() == AST::String) {
                buildPlatforms << QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue());
              }
            }
          }
          optionLayout->addWidget(new QLabel(optionName));

          QLineEdit *spinBox = new QLineEdit(optionWidget);
          QPushButton *browseButton = new QPushButton("...", optionWidget);
          QObject::connect(
              browseButton, &QPushButton::clicked, this, [&, spinBox]() {
                QString fileName = QFileDialog::getExistingDirectory(
                    this, tr("Find Files"), QDir::currentPath());
                qDebug() << fileName;
                if (fileName.size() > 0) {
                  spinBox->setText(fileName);
                }
              });

          if (systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
              systemConfig.frameworkConfigurations["all"].end()) {
            spinBox->setText(QString::fromStdString(std::get<std::string>(
                systemConfig
                    .frameworkConfigurations["all"][optionDecl->getName()])));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::String) {
              spinBox->setText(QString::fromStdString(
                  std::static_pointer_cast<ValueNode>(defaultNode)
                      ->getStringValue()));
            }
          }
          optionLayout->addWidget(spinBox);
          optionLayout->addWidget(browseButton);

          ASTNode metaNode = optionDecl->getPropertyValue("meta");
          if (metaNode && metaNode->getNodeType() == AST::String) {
            optionWidget->setToolTip(QString::fromStdString(
                std::static_pointer_cast<ValueNode>(metaNode)
                    ->getStringValue()));
          }
          lineEdits[QString::fromStdString(optionDecl->getName())] = spinBox;
        } else if (type == "stringListOption") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          optionLayout->addWidget(new QLabel(optionName));

          QComboBox *combo = new QComboBox(optionWidget);

          ASTNode possiblesNode =
              optionDecl->getPropertyValue("possibleValues");

          QString defaultValue;
          ASTNode defaultNode = optionDecl->getPropertyValue("default");
          if (systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
              systemConfig.frameworkConfigurations["all"].end()) {
            defaultValue = QString::fromStdString(std::get<std::string>(
                systemConfig
                    .frameworkConfigurations["all"][optionDecl->getName()]));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::String) {
              defaultValue = QString::fromStdString(
                  std::static_pointer_cast<ValueNode>(defaultNode)
                      ->getStringValue());
            }
          }
          int defaultIndex = 0;
          if (possiblesNode && possiblesNode->getNodeType() == AST::List) {
            std::shared_ptr<ListNode> list =
                std::static_pointer_cast<ListNode>(possiblesNode);
            for (ASTNode member : list->getChildren()) {
              if (member && member->getNodeType() == AST::String) {
                combo->addItem(QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue()));
                if (defaultValue ==
                    QString::fromStdString(
                        std::static_pointer_cast<ValueNode>(member)
                            ->getStringValue())) {
                  defaultIndex = combo->count() - 1;
                }
              }
            }
          }
          if (defaultNode ||
              systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
                  systemConfig.frameworkConfigurations["all"].end()) {
            combo->setCurrentIndex(defaultIndex);
          }
          optionLayout->addWidget(combo);
          comboBoxes[QString::fromStdString(optionDecl->getName())] = combo;
        } else if (type == "intListOption") {
          QString optionName = "----";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          optionLayout->addWidget(new QLabel(optionName));

          QComboBox *combo = new QComboBox(optionWidget);

          ASTNode possiblesNode =
              optionDecl->getPropertyValue("possibleValues");
          ASTNode defaultNode = optionDecl->getPropertyValue("default");
          int defaultValue = 0;
          if (systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
              systemConfig.frameworkConfigurations["all"].end()) {

            defaultValue = std::get<int64_t>(
                systemConfig
                    .frameworkConfigurations["all"][optionDecl->getName()]);
          } else { // Use default
            if (defaultNode && defaultNode->getNodeType() == AST::Int) {
              defaultValue = std::static_pointer_cast<ValueNode>(defaultNode)
                                 ->getIntValue();
            }
          }
          int defaultIndex = 0;
          if (possiblesNode && possiblesNode->getNodeType() == AST::List) {
            std::shared_ptr<ListNode> list =
                std::static_pointer_cast<ListNode>(possiblesNode);
            for (ASTNode member : list->getChildren()) {
              if (member && member->getNodeType() == AST::Int) {
                combo->addItem(
                    QString::number(std::static_pointer_cast<ValueNode>(member)
                                        ->getIntValue()));
                if (defaultValue == std::static_pointer_cast<ValueNode>(member)
                                        ->getIntValue()) {
                  defaultIndex = combo->count() - 1;
                }
              }
            }
          }
          if (defaultNode ||
              systemConfig.frameworkConfigurations["all"].find(
                  optionDecl->getName()) !=
                  systemConfig.frameworkConfigurations["all"].end()) {
            combo->setCurrentIndex(defaultIndex);
          }
          optionLayout->addWidget(combo);
          intComboBoxes[QString::fromStdString(optionDecl->getName())] = combo;
        } else if (type == "intOverride") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          QStringList buildPlatforms;
          ASTNode buildPlatformsNode =
              optionDecl->getPropertyValue("buildPlatforms");
          if (buildPlatformsNode &&
              buildPlatformsNode->getNodeType() == AST::List) {
            for (ASTNode member : buildPlatformsNode->getChildren()) {
              if (member->getNodeType() == AST::String) {
                buildPlatforms << QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue());
              }
            }
          }
          optionLayout->addWidget(new QLabel(optionName));

          ASTNode maxNode = optionDecl->getPropertyValue("maximum");
          ASTNode minNode = optionDecl->getPropertyValue("minimum");
          QSpinBox *spinBox = new QSpinBox(optionWidget);

          if (minNode && minNode->getNodeType() == AST::Int) {
            spinBox->setMinimum(
                std::static_pointer_cast<ValueNode>(minNode)->getIntValue());
          }
          if (maxNode && maxNode->getNodeType() == AST::Int) {
            spinBox->setMaximum(
                std::static_pointer_cast<ValueNode>(maxNode)->getIntValue());
          }

          if (systemConfig.overrides["all"].find(optionDecl->getName()) !=
              systemConfig.overrides["all"].end()) {

            spinBox->setValue(std::get<int64_t>(
                systemConfig.overrides["all"][optionDecl->getName()]));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::Int) {
              spinBox->setValue(std::static_pointer_cast<ValueNode>(defaultNode)
                                    ->getIntValue());
            }
          }
          optionLayout->addWidget(spinBox);

          ASTNode metaNode = optionDecl->getPropertyValue("meta");
          if (metaNode && metaNode->getNodeType() == AST::String) {
            optionWidget->setToolTip(QString::fromStdString(
                std::static_pointer_cast<ValueNode>(metaNode)
                    ->getStringValue()));
          }
          overrideSpinBoxes[QString::fromStdString(optionDecl->getName())] =
              spinBox;
        } else if (type == "stringListOverride") {
          QString optionName = "";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          optionLayout->addWidget(new QLabel(optionName));

          QComboBox *combo = new QComboBox(optionWidget);

          ASTNode possiblesNode =
              optionDecl->getPropertyValue("possibleValues");

          QString defaultValue;
          ASTNode defaultNode = optionDecl->getPropertyValue("default");
          if (systemConfig.overrides["all"].find(optionDecl->getName()) !=
              systemConfig.overrides["all"].end()) {

            defaultValue = QString::fromStdString(std::get<std::string>(
                systemConfig.overrides["all"][optionDecl->getName()]));
          } else { // Use default
            ASTNode defaultNode = optionDecl->getPropertyValue("default");
            if (defaultNode && defaultNode->getNodeType() == AST::String) {
              defaultValue = QString::fromStdString(
                  std::static_pointer_cast<ValueNode>(defaultNode)
                      ->getStringValue());
            }
          }
          int defaultIndex = 0;
          if (possiblesNode && possiblesNode->getNodeType() == AST::List) {
            std::shared_ptr<ListNode> list =
                std::static_pointer_cast<ListNode>(possiblesNode);
            for (ASTNode member : list->getChildren()) {
              if (member && member->getNodeType() == AST::String) {
                combo->addItem(QString::fromStdString(
                    std::static_pointer_cast<ValueNode>(member)
                        ->getStringValue()));
                if (defaultValue ==
                    QString::fromStdString(
                        std::static_pointer_cast<ValueNode>(member)
                            ->getStringValue())) {
                  defaultIndex = combo->count() - 1;
                }
              }
            }
          }
          if (defaultNode ||
              systemConfig.overrides["all"].find(optionDecl->getName()) !=
                  systemConfig.overrides["all"].end()) {
            combo->setCurrentIndex(defaultIndex);
          }
          optionLayout->addWidget(combo);
          overrideComboBoxes[QString::fromStdString(optionDecl->getName())] =
              combo;
        } else if (type == "intListOverride") {
          QString optionName = "----";
          ASTNode nameNode = optionDecl->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            optionName = QString::fromStdString(
                std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue());
          }
          optionLayout->addWidget(new QLabel(optionName));

          QComboBox *combo = new QComboBox(optionWidget);

          ASTNode possiblesNode =
              optionDecl->getPropertyValue("possibleValues");
          ASTNode defaultNode = optionDecl->getPropertyValue("default");
          int defaultValue = 0;
          if (systemConfig.overrides["all"].find(optionDecl->getName()) !=
              systemConfig.overrides["all"].end()) {

            defaultValue = std::get<int64_t>(
                systemConfig.overrides["all"][optionDecl->getName()]);
          } else { // Use default
            if (defaultNode && defaultNode->getNodeType() == AST::Int) {
              defaultValue = std::static_pointer_cast<ValueNode>(defaultNode)
                                 ->getIntValue();
            }
          }
          int defaultIndex = 0;
          if (possiblesNode && possiblesNode->getNodeType() == AST::List) {
            std::shared_ptr<ListNode> list =
                std::static_pointer_cast<ListNode>(possiblesNode);
            for (ASTNode member : list->getChildren()) {
              if (member && member->getNodeType() == AST::Int) {
                combo->addItem(
                    QString::number(std::static_pointer_cast<ValueNode>(member)
                                        ->getIntValue()));
                if (defaultValue == std::static_pointer_cast<ValueNode>(member)
                                        ->getIntValue()) {
                  defaultIndex = combo->count() - 1;
                }
              }
            }
          }
          if (defaultNode ||
              systemConfig.overrides["all"].find(optionDecl->getName()) !=
                  systemConfig.overrides["all"].end()) {
            combo->setCurrentIndex(defaultIndex);
          }
          optionLayout->addWidget(combo);
          overrideIntComboBoxes[QString::fromStdString(optionDecl->getName())] =
              combo;
        }
      }
      optionWidget->setLayout(optionLayout);
      groupLayout->addWidget(optionWidget);
    }
    groupBox->setLayout(groupLayout);
    mainLayout->addTab(groupBox, QString::fromStdString(groupName));
  }

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()), &optionsDialog, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), &optionsDialog, SLOT(reject()));

  QVBoxLayout *outerLayout = new QVBoxLayout;
  outerLayout->addWidget(mainLayout);
  outerLayout->addWidget(buttonBox);
  optionsDialog.setLayout(outerLayout);
  optionsDialog.exec();

  if (optionsDialog.result() == QDialog::Accepted) {
    for (auto comboBoxInfo = comboBoxes.constBegin();
         comboBoxInfo != comboBoxes.constEnd(); ++comboBoxInfo) {

      systemConfig
          .frameworkConfigurations["all"][comboBoxInfo.key().toStdString()] =
          comboBoxInfo.value()->currentText().toStdString();
    }
    for (auto comboBoxInfo = intComboBoxes.constBegin();
         comboBoxInfo != intComboBoxes.constEnd(); ++comboBoxInfo) {
      systemConfig
          .frameworkConfigurations["all"][comboBoxInfo.key().toStdString()] =
          comboBoxInfo.value()->currentText().toLongLong();
    }
    for (auto spinBoxInfo = spinBoxes.constBegin();
         spinBoxInfo != spinBoxes.constEnd(); ++spinBoxInfo) {
      systemConfig
          .frameworkConfigurations["all"][spinBoxInfo.key().toStdString()] =
          (int64_t)spinBoxInfo.value()->value();
    }
    for (auto lineEditInfo = lineEdits.constBegin();
         lineEditInfo != lineEdits.constEnd(); ++lineEditInfo) {
      qDebug() << lineEditInfo.key() << ".." << lineEditInfo.value()->text();
      systemConfig
          .frameworkConfigurations["all"][lineEditInfo.key().toStdString()] =
          lineEditInfo.value()->text().toStdString();
    }

    for (auto comboBoxInfo = overrideComboBoxes.constBegin();
         comboBoxInfo != overrideComboBoxes.constEnd(); ++comboBoxInfo) {
      systemConfig.overrides["all"][comboBoxInfo.key().toStdString()] =
          comboBoxInfo.value()->currentText().toStdString();
    }
    for (auto comboBoxInfo = overrideIntComboBoxes.constBegin();
         comboBoxInfo != overrideIntComboBoxes.constEnd(); ++comboBoxInfo) {
      systemConfig.overrides["all"][comboBoxInfo.key().toStdString()] =
          comboBoxInfo.value()->currentText().toLongLong();
    }
    for (auto spinBoxInfo = overrideSpinBoxes.constBegin();
         spinBoxInfo != overrideSpinBoxes.constEnd(); ++spinBoxInfo) {
      systemConfig.overrides["all"][spinBoxInfo.key().toStdString()] =
          (int64_t)spinBoxInfo.value()->value();
    }

    auto updateResourceConfig = [&](QString key, QVariant value) {
      bool found = false;
      for (auto frameworkConfig : systemConfig.resourceConfigurations) {
        for (auto resourceConfig : frameworkConfig.second) {
          if (resourceConfig->getName() == key.toStdString()) {
            auto valueNode = resourceConfig->getPropertyValue("value");
            if (valueNode) {
              if (valueNode->getNodeType() == AST::Int) {
                resourceConfig->replacePropertyValue(
                    "value", std::make_shared<ValueNode>((int64_t)value.toInt(),
                                                         __FILE__, __LINE__));
                found = true;
                break;
              } else if (valueNode->getNodeType() == AST::Real) {
                resourceConfig->replacePropertyValue(
                    "value", std::make_shared<ValueNode>(value.toDouble(),
                                                         __FILE__, __LINE__));
                found = true;
                break;
              } else if (valueNode->getNodeType() == AST::String) {
                resourceConfig->replacePropertyValue(
                    "value",
                    std::make_shared<ValueNode>(value.toString().toStdString(),
                                                __FILE__, __LINE__));
                found = true;
                break;
              } else {
                qDebug() << "ERROR unsupported data type in config";
              }
            } else {
              qDebug() << "ERROR resource config entry has no value property";
            }
          }
        }
      }
      if (!found) {
        auto properties = std::make_shared<ListNode>(__FILE__, __LINE__);
        if (value.type() == QVariant::Int) {
          properties->addChild(std::make_shared<PropertyNode>(
              "value",
              std::make_shared<ValueNode>((int64_t)value.toInt(), __FILE__,
                                          __LINE__),
              __FILE__, __LINE__));
        } else if (value.type() == QVariant::Double) {
          properties->addChild(std::make_shared<PropertyNode>(
              "value",
              std::make_shared<ValueNode>(value.toDouble(), __FILE__, __LINE__),
              __FILE__, __LINE__));
        } else if (value.type() == QVariant::String) {
          properties->addChild(std::make_shared<PropertyNode>(
              "value",
              std::make_shared<ValueNode>(value.toString().toStdString(),
                                          __FILE__, __LINE__),
              __FILE__, __LINE__));
        }
        auto newDecl = std::make_shared<DeclarationNode>(
            key.toStdString(), "constant", properties, __FILE__, __LINE__);
        systemConfig.resourceConfigurations["all"].push_back(newDecl);
      }
    };

    for (auto comboBoxInfo = resourceComboBoxes.constBegin();
         comboBoxInfo != resourceComboBoxes.constEnd(); ++comboBoxInfo) {
      updateResourceConfig(comboBoxInfo.key(),
                           comboBoxInfo.value()->currentText());
    }

    for (auto comboBoxInfo = resourceIntComboBoxes.constBegin();
         comboBoxInfo != resourceIntComboBoxes.constEnd(); ++comboBoxInfo) {
      updateResourceConfig(comboBoxInfo.key(),
                           comboBoxInfo.value()->currentText().toInt());
    }
    for (auto spinBoxInfo = resourceSpinBoxes.constBegin();
         spinBoxInfo != resourceSpinBoxes.constEnd(); ++spinBoxInfo) {

      updateResourceConfig(spinBoxInfo.key(), spinBoxInfo.value()->value());
    }
    for (auto lineEditInfo = resourceLineEdits.constBegin();
         lineEditInfo != resourceLineEdits.constEnd(); ++lineEditInfo) {
      updateResourceConfig(lineEditInfo.key(), lineEditInfo.value()->text());
    }

    // Write configuration to file
    CodeEditor *editor =
        static_cast<CodeEditor *>(ui->tabWidget->currentWidget());

    systemConfig.writeConfiguration(editor->filename().toStdString() +
                                    ".config");
  }
}

void ProjectWindow::resetCodeTimer() {
  m_codeModelTimer.stop();
  m_codeModelTimer.start();
}

void ProjectWindow::inspectorItemClicked(QTreeWidgetItem *item,
                                         int /*column*/) {
  QVariantList dataList = item->data(0, Qt::UserRole).toList();
  if (dataList.size() == 2) {
    QString fileName = dataList[0].toString();

    CodeEditor *editor =
        static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
    if (editor->filename() == fileName) {
      int line = dataList[1].toInt();
      if (line >= 0) {
        editor->gotoLine(line);
      }
    } else {
      loadFile(fileName);
      editor = static_cast<CodeEditor *>(ui->tabWidget->currentWidget());
      int line = dataList[1].toInt();
      if (line >= 0) {
        editor->gotoLine(line);
      }
    }
  }
}

void ProjectWindow::closeEvent(QCloseEvent *event) {
  if (maybeSave()) {
    writeSettings();
    for (auto builder : m_builders) {
      builder->run(false);
      delete builder;
    }
    m_builders.clear();
    event->accept();
  } else {
    event->ignore();
  }
  //    QWidget::closeEvent(event);
}

bool ProjectWindow::eventFilter(QObject *obj, QEvent *event) {
  Q_UNUSED(obj)
  if (event->type() == QEvent::FileOpen) {
    QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
    loadFile(openEvent->file());
    return true;
  }
  //    qDebug() << "Application event: " << event->type();
  return false;
}
