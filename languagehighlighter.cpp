#include <QDebug>

#include "languagehighlighter.h"

LanguageHighlighter::LanguageHighlighter(QObject *parent, UgenInterface *ugens) :
    QSyntaxHighlighter(parent), m_ugens_ptr(ugens)
{
}

void LanguageHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat myClassFormat;
    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setForeground(Qt::darkMagenta);
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
}
