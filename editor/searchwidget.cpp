

#include "searchwidget.h"
#include "ui_searchwidget.h"


SearchWidget::SearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWidget)
{
    ui->setupUi(this);
    // TODO implement replace strings
    ui->replaceLineEdit->hide();
    ui->replaceLabel->hide();
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

QString SearchWidget::searchString()
{
    return ui->findLineEdit->text();
}

QString SearchWidget::replaceString()
{
    return ui->replaceLineEdit->text();
}

void SearchWidget::setSearchString(QString query, bool focusQuery)
{
    ui->findLineEdit->setText(query);
    if (focusQuery) {
        ui->findLineEdit->setFocus();
    }
}

QPushButton *SearchWidget::getFindPreviousButton() {
    return ui->previousPushButton;
}

QPushButton *SearchWidget::getFindNextButton() {
    return ui->nextPushButton;
}
