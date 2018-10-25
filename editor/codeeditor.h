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

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <mutex>

#include <QPlainTextEdit>
#include <QList>
#include <QTimer>
#include <QPushButton>
#include <QAtomicInt>

#include "langerror.h"
#include "errormarker.h"
#include "tooltip.hpp"
#include "codemodel.hpp"
#include "autocompletemenu.hpp"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent, CodeModel *codeModel);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool isChanged();
    bool changedSinceParse();
    void markParsed();

    void setAutoComplete(bool enable);

    void setErrors(QList<LangError> errors);
    void setToolTipText(QString text);

    QString filename() const;
    void setFilename(const QString &filename);

    void find(QString query = "");
    void gotoLine(int line);

public slots:
    void markChanged(bool changed = true);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyReleaseEvent(QKeyEvent * event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void showButton();
    void hideButton();
    void helperButtonClicked();
    void mouseIdleTimeout();
    void insertAutoComplete();
    void updateAutoCompleteMenu(QString currentWord);

private:
    QWidget *m_lineNumberArea;
    CodeModel *m_codeModel;
    AutoCompleteMenu m_autoCompleteMenu;
    std::vector<std::shared_ptr<ErrorMarker>> m_errorMarkers;
    std::mutex m_markerLock;
    QTimer m_ButtonTimer;
    QTimer m_mouseIdleTimer;
    QAtomicInt m_changedSinceParse {1};

    // Properties
    QString m_filename;
    bool m_IndentTabs;
    bool m_autoComplete;

    QPushButton m_helperButton;
    ToolTip m_toolTip;

signals:
    void requestAssistant(QPoint point);

};



#endif // CODEEDITOR_H
