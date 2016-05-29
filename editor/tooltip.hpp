#ifndef TOOLTIP_HPP
#define TOOLTIP_HPP

#include <QLabel>

class ToolTip : public QLabel
{
    Q_OBJECT
public:
    ToolTip(QWidget *parent);

};

#endif // TOOLTIP_HPP
