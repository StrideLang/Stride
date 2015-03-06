#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QFont>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

    QFont font() const;
    void setFont(const QFont &font);

private:
    Ui::ConfigDialog *ui;
    QFont m_font;

private slots:
    void selectEditorFont();
};

#endif // CONFIGDIALOG_H
