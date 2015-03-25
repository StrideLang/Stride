#ifndef SAVECHANGEDDIALOG_H
#define SAVECHANGEDDIALOG_H

#include <QDialog>

namespace Ui {
class SaveChangedDialog;
}

class SaveChangedDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveChangedDialog(QWidget *parent = 0);
    ~SaveChangedDialog();

    void setListContents(QStringList fileNames);
    QList<int> getSelected();

private:
    Ui::SaveChangedDialog *ui;
};

#endif // SAVECHANGEDDIALOG_H
