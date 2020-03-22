#include <memory>

#include <QProcess>
#include <QtDebug>

#include "striderootmanagementdialog.h"
#include "ui_striderootmanagementdialog.h"

#import "ast.h"
#import "declarationnode.h"
#import "valuenode.h"

StriderootManagementDialog::StriderootManagementDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::StriderootManagementDialog) {
  ui->setupUi(this);
  QObject::connect(ui->elementTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
                   this, SLOT(itemClicked(QTreeWidgetItem *, int)));
  QObject::connect(ui->installButton, SIGNAL(clicked()), this,
                   SLOT(installFramework()));
}

StriderootManagementDialog::~StriderootManagementDialog() { delete ui; }

void StriderootManagementDialog::prepare() {
  ui->elementTree->clear();
  for (auto fw : m_frameworkNames) {
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->elementTree);
    item->setText(0, fw);
    ui->elementTree->insertTopLevelItem(0, item);
    item->setData(0, Qt::DisplayRole, QVariant(fw));
  }
}

void StriderootManagementDialog::itemClicked(QTreeWidgetItem *item,
                                             int column) {
  ui->detailsText->clear();
  ui->detailsText->setText(item->data(column, Qt::DisplayRole).toString());
}

void StriderootManagementDialog::installFramework() {
  auto currentItem = ui->elementTree->currentItem();
  if (currentItem) {
    auto frameworkName = currentItem->text(0);
    QString version = "1.0";
    auto frameworkRoot =
        m_strideRoot + "/frameworks/" + frameworkName + "/" + version;
    auto fileName = frameworkRoot + "/platformlib/Configuration.stride";
    auto configuration = AST::parseFile(fileName.toLatin1().constData());
    if (configuration) {
      for (auto node : configuration->getChildren()) {
        if (node->getNodeType() == AST::Declaration) {
          auto decl = std::static_pointer_cast<DeclarationNode>(node);
          auto installationNode = decl->getPropertyValue("installation");
          if (installationNode) {
            for (auto installDirectiveNode : installationNode->getChildren()) {
              if (installDirectiveNode->getNodeType() == AST::Declaration) {
                auto installDirective =
                    std::static_pointer_cast<DeclarationNode>(
                        installDirectiveNode);
                if (installDirective->getObjectType() == "installAction") {
                  // FIXME check for platform and language
                  auto commandNode =
                      installDirective->getPropertyValue("command");
                  auto dirNode =
                      installDirective->getPropertyValue("workingDirectory");
                  if (commandNode &&
                      commandNode->getNodeType() == AST::String && dirNode) {
                    QString workingDirectory = frameworkRoot;
                    if (dirNode->getNodeType() == AST::String) {
                      workingDirectory +=
                          "/" + QString::fromStdString(
                                    std::static_pointer_cast<ValueNode>(dirNode)
                                        ->getStringValue());
                    }
                    QProcess installProcess;
                    installProcess.setWorkingDirectory(workingDirectory);
                    auto command =
                        std::static_pointer_cast<ValueNode>(commandNode)
                            ->getStringValue();
                    qDebug() << "Running command: "
                             << QString::fromStdString(command);
                    installProcess.start(QString::fromStdString(command));
                    installProcess.waitForFinished();
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
