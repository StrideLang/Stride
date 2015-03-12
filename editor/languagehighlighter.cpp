#include <QDebug>

#include <QMutexLocker>

#include "languagehighlighter.h"

LanguageHighlighter::LanguageHighlighter(QObject *parent, UgenInterface *ugens) :
    QSyntaxHighlighter(parent), m_ugens_ptr(ugens)
{
    setFormatPreset(0);
    m_keywords << "none" << "on" << "off" << "streamRate"
               << "use" << "version";
}

void LanguageHighlighter::highlightBlock(const QString &text)
{
    QMutexLocker locker(&m_highlighterLock);
    QString pattern = "\\b[A-Z]\\w+\\b";

    QRegExp expression(pattern);
    int index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, m_formats["user"]);
        index = text.indexOf(expression, index + length);
    }

    pattern = "\\b\\w+\\s*:";

    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, m_formats["properties"]);
        index = text.indexOf(expression, index + length);
    }

    foreach(QString keyword, m_keywords) {
        pattern = QString("\\b%1\\b").arg(keyword);
        expression.setPattern(pattern);
        index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, m_formats["keywords"]);
            index = text.indexOf(expression, index + length);
        }
    }

    // Leave comments for last
    pattern = "\\#.*";
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, m_formats["comments"]);
        index = text.indexOf(expression, index + length);
    }
}

QMap<QString, QTextCharFormat> LanguageHighlighter::formats()
{
    QMutexLocker locker(&m_highlighterLock);
    return m_formats;
}

void LanguageHighlighter::setFormats(const QMap<QString, QTextCharFormat> &formats)
{
    m_highlighterLock.lock();
    m_formats = formats;
    m_highlighterLock.unlock();
    rehighlight();
}

void LanguageHighlighter::setFormatPreset(int index)
{
    QTextCharFormat keywordFormat;
    QTextCharFormat commentsFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat userFormat;
    QTextCharFormat propertiesFormat;
    QTextCharFormat builtinFormat;

    if (index == 0) {
        keywordFormat.setFontWeight(QFont::Bold);
        keywordFormat.setForeground(QColor("#ff9900"));
        keywordFormat.setBackground(Qt::white);

        commentsFormat.setFontWeight(QFont::Normal);
        commentsFormat.setForeground(Qt::black);
        commentsFormat.setBackground(Qt::green);

        typeFormat.setFontWeight(QFont::Normal);
        typeFormat.setForeground(QColor("#cc0000"));
        typeFormat.setBackground(Qt::white);

        userFormat.setFontWeight(QFont::Normal);
        userFormat.setForeground(Qt::blue);
        userFormat.setBackground(Qt::white);

        propertiesFormat.setFontWeight(QFont::Bold);
        propertiesFormat.setForeground(Qt::darkGreen);
        propertiesFormat.setBackground(Qt::white);

        builtinFormat.setFontWeight(QFont::Normal);
        builtinFormat.setForeground(Qt::red);
        builtinFormat.setBackground(Qt::white);
    } else {
        return;
    }

    m_formats["keywords"] = keywordFormat;
    m_formats["comments"] = commentsFormat;
    m_formats["type"] = typeFormat;
    m_formats["user"] = userFormat;
    m_formats["properties"] = propertiesFormat;
    m_formats["builtin"] = builtinFormat;

    emit currentHighlightingChanged(m_formats);
}

