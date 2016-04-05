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
