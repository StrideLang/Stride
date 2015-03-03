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
        if (m_ugens_ptr && m_ugens_ptr->isUgen(expression.capturedTexts()[0])) {
            setFormat(index, length, myClassFormat);
        }
        index = text.indexOf(expression, index + length);
    }

    myClassFormat.setFontWeight(QFont::Bold);
    myClassFormat.setForeground(Qt::red);
    pattern = "\\bIN_[0-9]+\\b";

    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, myClassFormat);
        index = text.indexOf(expression, index + length);
    }
    pattern = "\\bOUT_[0-9]+\\b";

    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, myClassFormat);
        index = text.indexOf(expression, index + length);
    }

    QStringList keywords;
    keywords << "label" << "split" << "merge" << "bus"
            << "mono" << "stereo" << "instrument"
            << "channels" << "polyphony" << "trigger" << "sustain"
            << "control" << "at";

    foreach(QString keyword, keywords) {
        pattern = QString("\\b%1\\b").arg(keyword);

        myClassFormat.setFontWeight(QFont::Normal);
        myClassFormat.setForeground(QColor(Qt::yellow).darker());
        expression.setPattern(pattern);
        index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, myClassFormat);
            index = text.indexOf(expression, index + length);
        }
    }

    // Leave comments for last
    myClassFormat.setFontWeight(QFont::Normal);
    myClassFormat.setForeground(Qt::black);
    myClassFormat.setBackground(Qt::darkGreen);
    pattern = "\\#.*";
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, myClassFormat);
        index = text.indexOf(expression, index + length);
    }
}
