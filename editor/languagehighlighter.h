#ifndef LANGUAGEHIGHLIGHTER_H
#define LANGUAGEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QMap>
#include <QMutex>

#include "ugeninterface.h"

class LanguageHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LanguageHighlighter(QObject *parent = 0, UgenInterface *ugens = 0);

    QMap<QString, QTextCharFormat> formats();
    void setFormats(const QMap<QString, QTextCharFormat> &formats);
public slots:
    void setFormatPreset(int index);

protected:
    virtual void highlightBlock(const QString &text);

signals:
    void currentHighlightingChanged(QMap<QString, QTextCharFormat> &formats);

public slots:

private:
    UgenInterface *m_ugens_ptr;
    QMap<QString, QTextCharFormat> m_formats;
    QStringList m_keywords;
    QMutex m_highlighterLock;
};

#endif // LANGUAGEHIGHLIGHTER_H
