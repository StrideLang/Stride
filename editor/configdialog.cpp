#include <QFontDialog>
#include <QColorDialog>

#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    connect(ui->fontPushButton, SIGNAL(released()),
            this, SLOT(selectEditorFont()));
    connect(ui->colorsTableWidget, SIGNAL(cellClicked(int,int)),
            this, SLOT(cellClicked(int,int)));
    connect(ui->colorsTableWidget, SIGNAL(cellChanged(int,int)),
            this, SLOT(cellChanged(int,int)));
    connect(ui->editorPresetComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setHighlighterPreset(int)));
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

bool ConfigDialog::autoComplete()
{
    return ui->autoCompleteCheckBox->isChecked();
}

void ConfigDialog::setAutoComplete(bool enabled)
{
    ui->autoCompleteCheckBox->setChecked(enabled);
}

QString ConfigDialog::platformRootPath()
{
    return ui->platformRootLineEdit->text();
}

void ConfigDialog::setPlatformRootPath(QString path)
{
    ui->platformRootLineEdit->setText(path);
}

QMap<QString, QTextCharFormat> ConfigDialog::highlighterFormats() const
{
    return m_highlighterFormats;
}

void ConfigDialog::setHighlighterFormats(QMap<QString, QTextCharFormat> &highlighterFormats)
{
    m_highlighterFormats = highlighterFormats;
    QStringList keys = m_highlighterFormats.keys();
    ui->colorsTableWidget->clearContents();
    ui->colorsTableWidget->setRowCount(keys.size());
    int row = 0;
    foreach(QString key, keys) {
        QTableWidgetItem *textItem = new QTableWidgetItem(key);
        ui->colorsTableWidget->setItem(row, 0, textItem);
        QTableWidgetItem *colorItem = new QTableWidgetItem();
        colorItem->setBackground(m_highlighterFormats[key].foreground());
        ui->colorsTableWidget->setItem(row, 1, colorItem);
        QTableWidgetItem *bgColorItem = new QTableWidgetItem();
        bgColorItem->setBackground(m_highlighterFormats[key].background());
        ui->colorsTableWidget->setItem(row, 2, bgColorItem);
        QTableWidgetItem *boldItem = new QTableWidgetItem();
        boldItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        boldItem->setCheckState(m_highlighterFormats[key].fontWeight() == QFont::Bold ? Qt::Checked : Qt::Unchecked);
        ui->colorsTableWidget->setItem(row, 3, boldItem);
        QTableWidgetItem *italicItem = new QTableWidgetItem();
        italicItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        italicItem->setCheckState(m_highlighterFormats[key].fontItalic() ? Qt::Checked : Qt::Unchecked);
        ui->colorsTableWidget->setItem(row, 4, italicItem);
        row++;
    }
}

void ConfigDialog::setHighlighterPreset(int index)
{
    emit requestHighlighterPreset(index);
}

void ConfigDialog::selectEditorFont()
{
    QFontDialog fontDialog(m_font, this);
    int result = fontDialog.exec();
    if(result == QDialog::Accepted) {
        setFont(fontDialog.selectedFont());
    }
}


void ConfigDialog::cellClicked(int row, int col)
{
    if(col == 1) {
        QTableWidgetItem *colorItem = ui->colorsTableWidget->item(row,col);
        QColorDialog colorDialog(colorItem->background().color(), this);
        if (colorDialog.exec() == QDialog::Accepted) {
            colorItem->setBackground(colorDialog.selectedColor());
            m_highlighterFormats[ui->colorsTableWidget->item(row,0)->text()].setForeground(colorDialog.selectedColor());
        }
    } else if(col == 2) {
        QTableWidgetItem *colorItem = ui->colorsTableWidget->item(row,col);
        QColorDialog colorDialog(colorItem->background().color(), this);
        if (colorDialog.exec() == QDialog::Accepted) {
            colorItem->setBackground(colorDialog.selectedColor());
            m_highlighterFormats[ui->colorsTableWidget->item(row,0)->text()].setBackground(colorDialog.selectedColor());
        }
    }
}

void ConfigDialog::cellChanged(int row, int col)
{
    if(col == 3) {
        QTableWidgetItem *item = ui->colorsTableWidget->item(row,col);
        m_highlighterFormats[ui->colorsTableWidget->item(row,0)->text()]
                .setFontWeight(item->checkState() == Qt::Checked ? QFont::Bold : QFont::Normal);
    } else if(col == 4) {
        QTableWidgetItem *item = ui->colorsTableWidget->item(row,col);
        m_highlighterFormats[ui->colorsTableWidget->item(row,0)->text()]
                .setFontItalic(item->checkState() == Qt::Checked);
    }
}


