#include <QDebug>

#include <QMutexLocker>
#include <QTextDocument>

#include "languagehighlighter.h"

LanguageHighlighter::LanguageHighlighter(QObject *parent) :
    QSyntaxHighlighter(parent)
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
//    while (index >= 0) {
//        int length = expression.matchedLength();
//        setFormat(index, length, m_formats["user"]);
//        index = text.indexOf(expression, index + length);
//    }

    // Properties/ports
    pattern ="([{,;\\s*]([a-z][a-zA-Z0-9_]*)[\\s]*:)";
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, m_formats["ports"]);
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

    foreach(QString blockType, m_blockTypes) {
        pattern = QString("\\b%1\\b").arg(blockType);
        expression.setPattern(pattern);
        index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, m_formats["type"]);
            index = text.indexOf(expression, index + length);
        }
    }

    foreach(QString objectName, m_builtinNames) {
        pattern = QString("\\b%1\\b").arg(objectName);
        expression.setPattern(pattern);
        index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, m_formats["builtin"]);
            index = text.indexOf(expression, index + length);
        }
    }

    foreach(QString functionName, m_functionNames) {
        pattern = QString("\\b%1\\b").arg(functionName);
        expression.setPattern(pattern);
        index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, m_formats["type"]);
            index = text.indexOf(expression, index + length);
        }
    }

    pattern = "\\\"(\\.|[^\"])*\\\"";
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, m_formats["strings"]);
        index = text.indexOf(expression, index + length);
    }

    pattern = "'(\\.|[^'])*'";
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
        int length = expression.matchedLength();
        setFormat(index, length, m_formats["strings"]);
        index = text.indexOf(expression, index + length);
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

void LanguageHighlighter::setBlockTypes(QStringList &blockTypes)
{
    m_highlighterLock.lock();
    m_blockTypes = blockTypes;
    m_highlighterLock.unlock();
    rehighlight();
}

void LanguageHighlighter::setFunctions(QStringList &functionNames)
{
    m_highlighterLock.lock();
    m_functionNames = functionNames;
    m_highlighterLock.unlock();
    rehighlight();

}

void LanguageHighlighter::setBuiltinObjects(QStringList &builtinNames)
{
    m_highlighterLock.lock();
    m_builtinNames = builtinNames;
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
    QTextCharFormat stringFormat;

    if (index == 0) {
        keywordFormat.setFontWeight(QFont::Bold);
        keywordFormat.setForeground(QColor("#ff9900"));
        keywordFormat.setBackground(Qt::white);

        commentsFormat.setFontWeight(QFont::Normal);
        commentsFormat.setForeground(Qt::black);
        commentsFormat.setBackground(Qt::green);

        typeFormat.setFontWeight(QFont::Bold);
        typeFormat.setForeground(QColor("#cc0000"));
        typeFormat.setBackground(Qt::white);

        userFormat.setFontWeight(QFont::Normal);
        userFormat.setForeground(Qt::blue);
        userFormat.setBackground(Qt::white);

        propertiesFormat.setFontWeight(QFont::Normal);
        propertiesFormat.setForeground(Qt::blue);
        propertiesFormat.setBackground(Qt::white);

        builtinFormat.setFontWeight(QFont::Bold);
        builtinFormat.setForeground(QColor("#cc0000"));
        builtinFormat.setBackground(Qt::white);

        stringFormat.setFontWeight(QFont::Bold);
        stringFormat.setForeground(QColor("#33CC00"));
        stringFormat.setBackground(Qt::white);
    } else {
        return;
    }

    m_formats["keywords"] = keywordFormat;
    m_formats["comments"] = commentsFormat;
    m_formats["type"] = typeFormat;
    m_formats["user"] = userFormat;
    m_formats["ports"] = propertiesFormat;
    m_formats["builtin"] = builtinFormat;
    m_formats["strings"] = stringFormat;

    emit currentHighlightingChanged(m_formats);
}

