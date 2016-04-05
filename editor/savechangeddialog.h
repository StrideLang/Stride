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

public slots:
    virtual int exec();

private slots:
    void selectionChanged();
    void dontSaveClicked();

private:
    Ui::SaveChangedDialog *ui;
    bool m_dontSave;
};

#endif // SAVECHANGEDDIALOG_H
