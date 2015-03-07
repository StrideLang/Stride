#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QFont>
#include <QTextCharFormat>


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

    QMap<QString, QTextCharFormat> highlighterFormats() const;
    void setHighlighterFormats(const QMap<QString, QTextCharFormat> &highlighterFormats);

private:
    Ui::ConfigDialog *ui;
    QFont m_font;
    QMap<QString, QTextCharFormat> m_highlighterFormats;

private slots:
    void selectEditorFont();
    void cellClicked(int row, int col);
    void cellChanged(int row, int col);
};



#endif // CONFIGDIALOG_H
