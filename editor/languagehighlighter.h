#ifndef LANGUAGEHIGHLIGHTER_H
#define LANGUAGEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QMap>

#include "ugeninterface.h"

class LanguageHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LanguageHighlighter(QObject *parent = 0, UgenInterface *ugens = 0);

protected:
    virtual void highlightBlock(const QString &text);

signals:

public slots:

private:
    UgenInterface *m_ugens_ptr;
    QMap<QString, QTextCharFormat> m_formats;
    QStringList m_keywords;
};

#endif // LANGUAGEHIGHLIGHTER_H
