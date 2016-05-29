#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPushButton>

namespace Ui {
class SearchWidget;
}

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);
    ~SearchWidget();

    QString searchString();
    QString replaceString();

    void setSearchString(QString query, bool focusQuery = false);

    QPushButton *getFindPreviousButton();
    QPushButton *getFindNextButton();

private:
    Ui::SearchWidget *ui;
};

#endif // SEARCHWIDGET_H
