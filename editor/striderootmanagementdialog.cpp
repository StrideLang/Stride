#include <memory>

#include <QDir>
#include <QProcess>
#include <QtDebug>

#import "strideframework.hpp"

#include "striderootmanagementdialog.h"
#include "ui_striderootmanagementdialog.h"

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
  for (auto fwName : m_frameworkNames) {
    auto versions = QDir(m_strideRoot + "/frameworks/" + fwName)
                        .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->elementTree);
    QMap<QString, QVariant> details;

    for (auto version : versions) {
      StrideFramework fw(m_strideRoot.toStdString(), fwName.toStdString(),
                         version.toStdString(), "", "");
      QString text = fwName + "\n" + "Version " + version + "\n";
      text += QString::fromStdString(fw.getPlatformDetails());
      details[version] = text;
    }

    item->setText(0, fwName);
    ui->elementTree->insertTopLevelItem(0, item);
    item->setData(0, Qt::UserRole, QVariant(details));
  }
}

void StriderootManagementDialog::itemClicked(QTreeWidgetItem *item,
                                             int column) {
  ui->detailsText->clear();

  ui->versionComboBox->clear();
  QMapIterator<QString, QVariant> i(item->data(column, Qt::UserRole).toMap());
  if (i.hasNext()) {
    i.next();
    ui->detailsText->setText(i.value().toString());
    ui->versionComboBox->addItem(i.key(), i.value());
  }
  while (i.hasNext()) {
    i.next();
    ui->versionComboBox->addItem(i.key(), i.value());
  }

  void (QComboBox::*mySignal)(const QString &text) = &QComboBox::activated;
  connect(ui->versionComboBox, mySignal,
          [this, item, column](const QString &text) {
            ui->detailsText->setText(
                item->data(column, Qt::UserRole).toMap()[text].toString());
          });
}

void StriderootManagementDialog::installFramework() {
  auto currentItem = ui->elementTree->currentItem();
  if (currentItem) {
    StrideFramework fw(
        m_strideRoot.toStdString(), currentItem->text(0).toStdString(),
        ui->versionComboBox->currentText().toStdString(), "", "");
    fw.installFramework();
  }
}
