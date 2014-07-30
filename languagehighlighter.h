#ifndef LANGUAGEHIGHLIGHTER_H
#define LANGUAGEHIGHLIGHTER_H

#include <QSyntaxHighlighter>

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
};

#endif // LANGUAGEHIGHLIGHTER_H
