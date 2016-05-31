#ifndef LANGUAGEHIGHLIGHTER_H
#define LANGUAGEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QMap>
#include <QMutex>

class LanguageHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LanguageHighlighter(QObject *parent = 0);

    QMap<QString, QTextCharFormat> formats();
    void setFormats(const QMap<QString, QTextCharFormat> &formats);
    void setBlockTypes(QStringList blockTypes);
    void setFunctions(QStringList functionNames);
    void setBuiltinObjects(QStringList builtinNames);

public slots:
    void setFormatPreset(int index);

protected:
    virtual void highlightBlock(const QString &text);

signals:
    void currentHighlightingChanged(QMap<QString, QTextCharFormat> &formats);

public slots:

private:
    QMap<QString, QTextCharFormat> m_formats;
    QStringList m_keywords;
    QStringList m_blockTypes;
    QStringList m_functionNames;
    QStringList m_builtinNames;
    QMutex m_highlighterLock;
};

#endif // LANGUAGEHIGHLIGHTER_H
