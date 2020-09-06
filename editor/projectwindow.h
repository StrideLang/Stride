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

#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QFile>
#include <QMainWindow>
#include <QMenu>
#include <QTimer>
#include <QTreeWidgetItem>

#include "builder.h"
#include "codemodel.hpp"
#include "languagehighlighter.h"
#include "searchwidget.h"
#include "systemconfiguration.hpp"

namespace Ui {
class ProjectWindow;
}

class ProjectWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit ProjectWindow(QWidget *parent = nullptr);
  ~ProjectWindow();

  void initialize(bool resetOpenFiles = false);

public slots:
  void setEditorText(QString code);
  bool saveFile(int index = -1);
  bool saveFileAs(int index = -1);
  void closeTab(int index = -1);
  void loadFile();
  void loadFile(QString fileName);
  void openOptionsDialog();
  void openManageStriderootDialog();
  void openManageLocalDialog();
  void openGeneratedDir();
  void cleanProject();
  void updateCodeAnalysis(bool force = false);
  void newFile();
  void markModified();
  void configureSystem();
  void resetCodeTimer();
  void inspectorItemClicked(QTreeWidgetItem *item, int column);

protected:
  virtual void closeEvent(QCloseEvent *event);
  virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
  bool build();
  void deploy();
  void run(bool pressed);
  void stop();
  void programStopped();
  void tabChanged(int index);
  bool maybeSave();
  void showDocumentation();
  void followSymbol();
  void openRecent();

  // Editor
  void commentSection();
  void uncomment();
  void showHelperMenu(QPoint where);
  void insertText(QString text = "");
  void find(QString query = "");
  void findNext();
  void findPrevious();

  void printConsoleText(QString text);
  void printConsoleError(QString text);

private:
  void connectActions();
  void connectShortcuts();
  void prepareMenus();

  void readSettings(bool resetOpenFiles = false);
  void writeSettings();

  void updateEditorSettings();

  const uint16_t m_maxRecentFiles = 20;
  QList<QAction *> m_recentFilesActions;
  QStringList m_recentFilePaths;
  void adjustForCurrentFile(const QString &filePath);
  void updateRecentActionList();

  SystemConfiguration readProjectConfiguration(string filename);

  void fillInspectorTree();
  QTreeWidgetItem *createTreeItem(ASTNode inputNode);

  Ui::ProjectWindow *ui;

  QScopedPointer<SearchWidget> m_searchWidget;

  LanguageHighlighter *m_highlighter;

  //    QString m_platformsRootDir;
  QMap<QString, QVariant> m_options;
  QMap<QString, QVariant> m_environment;
  QMap<QString, QVariant> m_guiOptions;
  QTimer m_codeModelTimer;
  CodeModel m_codeModel;
  QFont m_font;
  std::vector<Builder *> m_builders;
  QMenu m_helperMenu;

  bool m_startingUp;
};

#endif // PROJECTWINDOW_H
