
#include <QPainter>
#include <QLabel>

#include "errormarker.h"

ErrorMarker::ErrorMarker(QWidget *parent, int lineNumber, QString text) :
    QWidget(parent) , m_lineNumber(lineNumber), m_expand(false), m_errorText(text)
{
    m_text = new QLabel(m_errorText, this);
    m_text->move(this->x(), this->y());
    m_text->hide();
    m_text->adjustSize();
//    this->setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
//    m_text->resize(100, 100);

//    m_text->setWindowFlags(Qt::ToolTip);
}

ErrorMarker::~ErrorMarker()
{

}

void ErrorMarker::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
//    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);
    painter.drawRect(0, 0, 20, this->height());
    if (m_expand) {
        painter.drawRect(0, 0, 20, this->height());
    } else {
        painter.drawRect(0, 0, 100, this->height());
    }
}

void ErrorMarker::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_text->show();
    m_expand = true;
}


void ErrorMarker::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_text->hide();
    m_expand = false;
}

