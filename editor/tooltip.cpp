#include "tooltip.hpp"

ToolTip::ToolTip(QWidget *parent) : QLabel(parent)
{
    setText("Tooltip");
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    setWordWrap(true);
}
