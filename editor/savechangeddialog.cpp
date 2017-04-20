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

#include "savechangeddialog.h"
#include "ui_savechangeddialog.h"

#include <QPushButton>

SaveChangedDialog::SaveChangedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveChangedDialog),
    m_dontSave(false)
{
    ui->setupUi(this);
    QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Save);
    button->setDefault(true);
    QObject::connect(ui->listWidget,SIGNAL(itemSelectionChanged()),
            this, SLOT(selectionChanged()));

    ui->listWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    QObject::connect(ui->buttonBox->button(QDialogButtonBox::Discard), SIGNAL(released()),
                     this, SLOT(dontSaveClicked()));
}

SaveChangedDialog::~SaveChangedDialog()
{
    delete ui;
}

void SaveChangedDialog::setListContents(QStringList fileNames)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(fileNames);
    ui->listWidget->selectAll();
}

QList<int> SaveChangedDialog::getSelected()
{
    QList<int> selected;
    if (result() == QDialog::Rejected) {
        return selected;
    }
    foreach(QListWidgetItem *item, ui->listWidget->selectedItems()) {
        selected.append(ui->listWidget->row(item));
    }
    return selected;
}

int SaveChangedDialog::exec()
{
    int ret = QDialog::exec();
    return (m_dontSave ? 100 :ret);
}

void SaveChangedDialog::selectionChanged()
{
    QList<QListWidgetItem *> selected = ui->listWidget->selectedItems();
    if (selected.size() == 0) {
        ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Quit"));
    } else {
        ui->buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save"));
    }
}

void SaveChangedDialog::dontSaveClicked()
{
    m_dontSave = true;
    this->reject();
}
