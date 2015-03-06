#include <QFontDialog>

#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    connect(ui->fontPushButton, SIGNAL(released()),
            this, SLOT(selectEditorFont()));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

QFont ConfigDialog::font() const
{
    return m_font;
}

void ConfigDialog::setFont(const QFont &font)
{
    QString fontText = font.family() + " " + QString::number(font.pointSize());
    ui->fontPushButton->setText(fontText);
    m_font = font;
}


void ConfigDialog::selectEditorFont()
{
    QFontDialog fontDialog(m_font, this);
    int result = fontDialog.exec();
    if(result == QDialog::Accepted) {
        setFont(fontDialog.selectedFont());
    }
}
