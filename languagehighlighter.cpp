#include <QDebug>

#include "languagehighlighter.h"

LanguageHighlighter::LanguageHighlighter(QObject *parent, UgenInterface *ugens) :
    QSyntaxHighlighter(parent), m_ugens_ptr(ugens)
{
}

void LanguageHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Normal);
    myClassFormat.setForeground(Qt::blue);
    QString pattern = "\\b[A-Z][a-z]+\\b";

    QRegExp expression(pattern);
    int index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        if (m_ugens_ptr->isUgen(expression.capturedTexts()[0])) {
            setFormat(index, length, myClassFormat);
        }
        index = text.indexOf(expression, index + length);
    }

    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setForeground(Qt::red);
    pattern = "\\bDAC_[0-9]+\\b";

    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, myClassFormat);
        index = text.indexOf(expression, index + length);
    }
    pattern = "\\bADC_[0-9]+\\b";

    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, myClassFormat);
        index = text.indexOf(expression, index + length);
    }
}
