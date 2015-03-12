#include <QDebug>

#include <QMutexLocker>

#include "languagehighlighter.h"

LanguageHighlighter::LanguageHighlighter(QObject *parent, UgenInterface *ugens) :
    QSyntaxHighlighter(parent), m_ugens_ptr(ugens)
{
    QTextCharFormat keywordFormat;
    keywordFormat.setFontWeight(QFont::Normal);
    keywordFormat.setForeground(QColor(Qt::yellow).darker());
    keywordFormat.setBackground(Qt::white);
    m_formats["keywords"] = keywordFormat;

    QTextCharFormat commentsFormat;
    commentsFormat.setFontWeight(QFont::Normal);
    commentsFormat.setForeground(Qt::black);
    commentsFormat.setBackground(Qt::green);
    m_formats["comments"] = commentsFormat;

    QTextCharFormat typeFormat;
    typeFormat.setFontWeight(QFont::Normal);
    typeFormat.setForeground(Qt::black);
    typeFormat.setBackground(Qt::green);
    m_formats["type"] = typeFormat;

    QTextCharFormat userFormat;
    userFormat.setFontWeight(QFont::Normal);
    userFormat.setForeground(Qt::blue);
    userFormat.setBackground(Qt::white);
    m_formats["user"] = userFormat;

    QTextCharFormat propertiesFormat;
    propertiesFormat.setFontWeight(QFont::Bold);
    propertiesFormat.setForeground(Qt::darkGreen);
    propertiesFormat.setBackground(Qt::white);
    m_formats["properties"] = propertiesFormat;

    QTextCharFormat builtinFormat;
    builtinFormat.setFontWeight(QFont::Normal);
    builtinFormat.setForeground(Qt::red);
    builtinFormat.setBackground(Qt::white);
    m_formats["builtin"] = builtinFormat;

    m_keywords << "none" << "on" << "off" << "streamRate";
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

