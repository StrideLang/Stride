#include <memory>

#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QtDebug>

#include "strideframework.hpp"

#include "striderootmanagementdialog.h"
#include "ui_striderootmanagementdialog.h"

StriderootManagementDialog::StriderootManagementDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::StriderootManagementDialog) {
  ui->setupUi(this);
  QObject::connect(ui->elementTree, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
                   this, SLOT(itemClicked(QTreeWidgetItem *, int)));
  QObject::connect(ui->systemTreeWidget,
                   SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
                   SLOT(itemClicked(QTreeWidgetItem *, int)));
  QObject::connect(ui->installButton, SIGNAL(clicked()), this,
                   SLOT(installFramework()));

  QObject::connect(ui->striderootBrowseButton, &QToolButton::clicked,
                   [this](bool /*clicked*/) {
                     auto dir = QFileDialog::getExistingDirectory(
                         this, "strideroot", this->m_strideRoot);
                     if (dir.size() > 0) {
                       this->m_strideRoot = dir;
                       this->prepare();
                     }
                   });
}

StriderootManagementDialog::~StriderootManagementDialog() { delete ui; }

QString StriderootManagementDialog::platformRootPath() {
  return ui->strideRootLineEdit->text();
}

void StriderootManagementDialog::setPlatformRootPath(QString path) {
  ui->strideRootLineEdit->setText(path);
}

void StriderootManagementDialog::prepare() {
  ui->elementTree->clear();
  m_frameworkNames = QDir(m_strideRoot + "/frameworks")
                         .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (auto fwName : m_frameworkNames) {
    auto versions = QDir(m_strideRoot + "/frameworks/" + fwName)
                        .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QMap<QString, QVariant> details;

    for (auto version : versions) {
      if (QFile::exists(m_strideRoot + "/frameworks/" + fwName + "/" + version +
                        "/platformlib/Configuration.stride")) {
        StrideFramework fw(m_strideRoot.toStdString(), fwName.toStdString(),
                           version.toStdString(), "", "");
        QString text = fwName + "\n" + "Version " + version + "\n";
        text += QString::fromStdString(fw.getPlatformDetails());
        details[version] = text;
      }
    }

    if (details.size() > 0) {
      QTreeWidgetItem *item = new QTreeWidgetItem(ui->elementTree);
      item->setText(0, fwName);
      ui->elementTree->insertTopLevelItem(0, item);
      item->setData(0, Qt::UserRole, QVariant(details));
    }
  }

  m_systemNames = QDir(m_strideRoot + "/systems")
                      .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (auto sysName : m_systemNames) {
    auto versions = QDir(m_strideRoot + "/systems/" + sysName)
                        .entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QMap<QString, QVariant> details;

    for (auto version : versions) {
      if (QFile::exists(m_strideRoot + "/systems/" + sysName + "/" + version +
                        "/System.stride")) {
        StrideFramework fw(m_strideRoot.toStdString(), sysName.toStdString(),
                           version.toStdString(), "", "");
        QString text = sysName + "\n" + "Version " + version + "\n";
        text += QString::fromStdString(fw.getPlatformDetails());
        details[version] = text;
      }
    }

    if (details.size() > 0) {
      QTreeWidgetItem *item = new QTreeWidgetItem(ui->systemTreeWidget);
      item->setText(0, sysName);
      ui->systemTreeWidget->insertTopLevelItem(0, item);
      item->setData(0, Qt::UserRole, QVariant(details));
    }
  }

  ui->strideRootLineEdit->setText(m_strideRoot);
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
