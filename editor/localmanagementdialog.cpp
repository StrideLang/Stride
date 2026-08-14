#include "localmanagementdialog.hpp"
#include "ui_localmanagementdialog.h"

#include <QAction>

LocalManagementDialog::LocalManagementDialog(QString strideRoot,
                                             QWidget *parent)
    : QDialog(parent), ui(new Ui::LocalManagementDialog),
      m_strideroot(strideRoot) {
  ui->setupUi(this);

  connect(ui->updateAllButton, &QToolButton::clicked, [&](bool) {
    toolManager.updateAllLocalConfigs();
    updateLocal();
    updateToolInfo();
  });

  connect(ui->toolsListWidget, &QListWidget::itemClicked,
          [&](QListWidgetItem *item) {
            // Update instance list
            ui->instanceListWidget->clear();
            for (auto entry : tools[item->text()]) {

              ui->instanceListWidget->addItem(entry);
            }
            if (ui->instanceListWidget->count() > 0) {
              ui->instanceListWidget->setCurrentRow(0);
            }

            updateToolInfo();
          });

  connect(ui->instanceListWidget, &QListWidget::itemClicked,
          [&](QListWidgetItem *item) { updateToolInfo(); });

  connect(ui->pathsListWidget, &QListWidget::itemClicked,
          [&](QListWidgetItem *item) {
            // Update instance list
            ui->pathOptionListWidget->clear();
            for (auto entry : paths[item->text()]) {
              ui->pathOptionListWidget->addItem(entry);
            }

            updateToolInfo();
          });

  updateLocal();
}

LocalManagementDialog::~LocalManagementDialog() { delete ui; }

void LocalManagementDialog::updateLocal() {
  toolManager.setStrideRoot(m_strideroot.toStdString());
  toolManager.readTemplates();
  toolManager.readLocalConfigs();

  tools.clear();
  for (auto localTool : toolManager.localTools) {
    if (!tools.keys().contains(QString::fromStdString(localTool.first))) {
      tools[QString::fromStdString(localTool.first)] = QList<QString>();
    }
    tools[QString::fromStdString(localTool.first)].push_back(
        QString::fromStdString(localTool.second));
  }
  //  ui->toolL
  ui->toolsListWidget->clear();
  QMapIterator<QString, QList<QString>> i(tools);
  while (i.hasNext()) {
    i.next();
    ui->toolsListWidget->addItem(i.key());
  }
  if (ui->toolsListWidget->count() > 0) {
    ui->toolsListWidget->setCurrentRow(0);
    ui->instanceListWidget->clear();
    for (auto entry : tools[ui->toolsListWidget->currentItem()->text()]) {
      ui->instanceListWidget->addItem(entry);
    }
    if (ui->instanceListWidget->count() > 0) {
      ui->instanceListWidget->setCurrentRow(0);
    }
  }

  // paths
  paths.clear();
  for (auto localPath : toolManager.localPaths) {
    if (!paths.keys().contains(QString::fromStdString(localPath.first))) {
      paths[QString::fromStdString(localPath.first)] = QList<QString>();
    }
    paths[QString::fromStdString(localPath.first)].push_back(
        QString::fromStdString(localPath.second));
  }

  ui->pathsListWidget->clear();
  QMapIterator<QString, QList<QString>> j(paths);
  while (j.hasNext()) {
    j.next();
    ui->pathsListWidget->addItem(j.key());
  }
  if (ui->pathsListWidget->count() > 0) {
    ui->pathsListWidget->setCurrentRow(0);
    ui->pathOptionListWidget->clear();
    for (auto entry : paths[ui->pathsListWidget->currentItem()->text()]) {
      ui->pathOptionListWidget->addItem(entry);
    }
    if (ui->pathOptionListWidget->count() > 0) {
      ui->pathOptionListWidget->setCurrentRow(0);
    }
  }
  updateToolInfo();
}

void LocalManagementDialog::updateToolInfo() {

  // Update instance list
  int row = ui->instanceListWidget->currentRow();
  ui->toolInfoTextEdit->clear();
  if (row >= 0) {
    QString toolInfoText;
    toolInfoText += ui->toolsListWidget->currentItem()->text() + "\n";

    toolInfoText +=
        "Executable: " + tools[ui->toolsListWidget->currentItem()->text()][row];
    ui->toolInfoTextEdit->setText(toolInfoText);
  }
}
