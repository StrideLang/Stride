
#include <QPainter>
#include <QLabel>

#include "errormarker.h"

ErrorMarker::ErrorMarker(QWidget *parent, int lineNumber, QString text) :
    QWidget(parent) , m_lineNumber(lineNumber), m_errorText(text)
{
    m_text = new QLabel(m_errorText, this);
    m_text->move(this->x(), this->y());
    m_text->hide();
//    m_text->adjustSize();
//    m_text->resize(100, 100);
}

ErrorMarker::~ErrorMarker()
{

}

void ErrorMarker::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
//    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);
    painter.drawRect(rect());
}

void ErrorMarker::enterEvent(QEvent *event)
{
    m_text->show();
}


void ErrorMarker::leaveEvent(QEvent *event)
{
    m_text->hide();
}

