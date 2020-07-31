#ifndef LOCALMANAGEMENTDIALOG_HPP
#define LOCALMANAGEMENTDIALOG_HPP

#include <QDialog>
#include <QMap>
#include <QString>

#include "toolmanager.hpp"

namespace Ui {
class LocalManagementDialog;
}

class LocalManagementDialog : public QDialog {
  Q_OBJECT

public:
  explicit LocalManagementDialog(QString strideRoot, QWidget *parent = nullptr);
  ~LocalManagementDialog();

  void updateLocal();

private:
  void updateToolInfo();

  Ui::LocalManagementDialog *ui;
  ToolManager toolManager;

  QMap<QString, QList<QString>> tools;
  QMap<QString, QList<QString>> paths;

  QString m_strideroot;
};

#endif // LOCALMANAGEMENTDIALOG_HPP
