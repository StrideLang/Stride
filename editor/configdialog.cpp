/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <QColorDialog>
#include <QFontDialog>

#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConfigDialog) {
  ui->setupUi(this);
  connect(ui->fontPushButton, SIGNAL(released()), this,
          SLOT(selectEditorFont()));
  connect(ui->colorsTableWidget, SIGNAL(cellClicked(int, int)), this,
          SLOT(cellClicked(int, int)));
  connect(ui->colorsTableWidget, SIGNAL(cellChanged(int, int)), this,
          SLOT(cellChanged(int, int)));
  connect(ui->editorPresetComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(setHighlighterPreset(int)));
}

ConfigDialog::~ConfigDialog() { delete ui; }

QFont ConfigDialog::font() const { return m_font; }

void ConfigDialog::setFont(const QFont &font) {
  QString fontText = font.family() + " " + QString::number(font.pointSize());
  ui->fontPushButton->setText(fontText);
  m_font = font;
}

bool ConfigDialog::autoComplete() {
  return ui->autoCompleteCheckBox->isChecked();
}

void ConfigDialog::setAutoComplete(bool enabled) {
  ui->autoCompleteCheckBox->setChecked(enabled);
}

QMap<QString, QTextCharFormat> ConfigDialog::highlighterFormats() const {
  return m_highlighterFormats;
}

void ConfigDialog::setHighlighterFormats(
    QMap<QString, QTextCharFormat> &highlighterFormats) {
  m_highlighterFormats = highlighterFormats;
  QStringList keys = m_highlighterFormats.keys();
  ui->colorsTableWidget->clearContents();
  ui->colorsTableWidget->setRowCount(keys.size());
  int row = 0;
  foreach (QString key, keys) {
    QTableWidgetItem *textItem = new QTableWidgetItem(key);
    ui->colorsTableWidget->setItem(row, 0, textItem);
    QTableWidgetItem *colorItem = new QTableWidgetItem();
    colorItem->setBackground(m_highlighterFormats[key].foreground());
    ui->colorsTableWidget->setItem(row, 1, colorItem);
    QTableWidgetItem *bgColorItem = new QTableWidgetItem();
    bgColorItem->setBackground(m_highlighterFormats[key].background());
    ui->colorsTableWidget->setItem(row, 2, bgColorItem);
    QTableWidgetItem *boldItem = new QTableWidgetItem();
    boldItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable |
                       Qt::ItemIsEnabled);
    boldItem->setCheckState(
        m_highlighterFormats[key].fontWeight() == QFont::Bold ? Qt::Checked
                                                              : Qt::Unchecked);
    ui->colorsTableWidget->setItem(row, 3, boldItem);
    QTableWidgetItem *italicItem = new QTableWidgetItem();
    italicItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable |
                         Qt::ItemIsEnabled);
    italicItem->setCheckState(
        m_highlighterFormats[key].fontItalic() ? Qt::Checked : Qt::Unchecked);
    ui->colorsTableWidget->setItem(row, 4, italicItem);
    row++;
  }
}

void ConfigDialog::setHighlighterPreset(int index) {
  emit requestHighlighterPreset(index);
}

void ConfigDialog::selectEditorFont() {
  QFontDialog fontDialog(m_font, this);
  int result = fontDialog.exec();
  if (result == QDialog::Accepted) {
    setFont(fontDialog.selectedFont());
  }
}

void ConfigDialog::cellClicked(int row, int col) {
  if (col == 1) {
    QTableWidgetItem *colorItem = ui->colorsTableWidget->item(row, col);
    QColorDialog colorDialog(colorItem->background().color(), this);
    if (colorDialog.exec() == QDialog::Accepted) {
      colorItem->setBackground(colorDialog.selectedColor());
      m_highlighterFormats[ui->colorsTableWidget->item(row, 0)->text()]
          .setForeground(colorDialog.selectedColor());
    }
  } else if (col == 2) {
    QTableWidgetItem *colorItem = ui->colorsTableWidget->item(row, col);
    QColorDialog colorDialog(colorItem->background().color(), this);
    if (colorDialog.exec() == QDialog::Accepted) {
      colorItem->setBackground(colorDialog.selectedColor());
      m_highlighterFormats[ui->colorsTableWidget->item(row, 0)->text()]
          .setBackground(colorDialog.selectedColor());
    }
  }
}

void ConfigDialog::cellChanged(int row, int col) {
  if (col == 3) {
    QTableWidgetItem *item = ui->colorsTableWidget->item(row, col);
    m_highlighterFormats[ui->colorsTableWidget->item(row, 0)->text()]
        .setFontWeight(item->checkState() == Qt::Checked ? QFont::Bold
                                                         : QFont::Normal);
  } else if (col == 4) {
    QTableWidgetItem *item = ui->colorsTableWidget->item(row, col);
    m_highlighterFormats[ui->colorsTableWidget->item(row, 0)->text()]
        .setFontItalic(item->checkState() == Qt::Checked);
  }
}
