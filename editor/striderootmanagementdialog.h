#ifndef STRIDEROOTMANAGEMENTDIALOG_H
#define STRIDEROOTMANAGEMENTDIALOG_H

#include <QDialog>

class QTreeWidgetItem;

namespace Ui {
class StriderootManagementDialog;
}

class StriderootManagementDialog : public QDialog {
  Q_OBJECT

public:
  explicit StriderootManagementDialog(QWidget *parent = nullptr);
  ~StriderootManagementDialog();

  QString m_strideRoot;

  QString platformRootPath();
  void setPlatformRootPath(QString path);

  void prepare();

public slots:
  void itemClicked(QTreeWidgetItem *item, int column);
  void installFramework();

private:
  Ui::StriderootManagementDialog *ui;
  QVector<QString> m_frameworkDetails;

  QStringList m_frameworkNames;
  QStringList m_systemNames;
};

#endif // STRIDEROOTMANAGEMENTDIALOG_H
