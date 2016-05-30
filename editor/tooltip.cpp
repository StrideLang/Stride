#include "tooltip.hpp"

ToolTip::ToolTip(QWidget *parent) : QLabel(parent)
{
//    setText("Tooltip");
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    setWordWrap(true);
    QFont font("Sans", 10);
    setFont(font);
    QPalette palette;
    palette.setColor(QPalette::Background, QColor("darkslategray"));
    palette.setColor(QPalette::Foreground, QColor("azure"));
    setPalette(palette);
    setMargin(5);
    setMaximumWidth(500);
}
