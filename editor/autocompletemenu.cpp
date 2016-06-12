#include "autocompletemenu.hpp"

AutoCompleteMenu::AutoCompleteMenu(QWidget *parent) :
    QMenu(parent)
{
    installEventFilter(parent);
}

AutoCompleteMenu::~AutoCompleteMenu()
{
}
