/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

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
