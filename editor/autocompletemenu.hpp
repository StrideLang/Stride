#ifndef AUTOCOMPLETEMENU_HPP
#define AUTOCOMPLETEMENU_HPP

#include <QMenu>


class AutoCompleteMenu : public QMenu
{
    Q_OBJECT
public:
    explicit AutoCompleteMenu(QWidget *parent = 0);
    ~AutoCompleteMenu();

    void setCurrentText(QString text);

private:
};

#endif // AUTOCOMPLETEMENU_HPP
