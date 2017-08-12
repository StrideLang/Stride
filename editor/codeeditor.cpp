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

#include <QPainter>
#include <QTextBlock>
#include <QDebug>

#include "codeeditor.h"
#include "linenumberarea.h"

CodeEditor::CodeEditor(QWidget *parent, CodeModel *codeModel) :
    QPlainTextEdit(parent),
    m_codeModel(codeModel),
    m_autoCompleteMenu(this),
    m_IndentTabs(true),
    m_helperButton(this), m_toolTip((QWidget*)this)
{
    setMouseTracking(true);
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    m_ButtonTimer.setInterval(1400);
    m_ButtonTimer.setSingleShot(true);
    m_mouseIdleTimer.setInterval(900);
    m_mouseIdleTimer.setSingleShot(true);

    m_helperButton.hide();
    m_helperButton.setText("+");

    connect(&m_ButtonTimer, SIGNAL(timeout()), this, SLOT(showButton()));
    connect(&m_mouseIdleTimer, SIGNAL(timeout()), this, SLOT(mouseIdleTimeout()));
    connect(&m_helperButton,SIGNAL(pressed()), this, SLOT(helperButtonClicked()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(hideButton()));

    m_toolTip.hide();
    setContextMenuPolicy(Qt::CustomContextMenu);
}

int CodeEditor::lineNumberAreaWidth()
{
//    int digits = 1;
//    int max = qMax(1, blockCount());
//    while (max >= 10) {
//        max /= 10;
//        ++digits;
//    }

    int digits = 4;
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

bool CodeEditor::isChanged()
{
    return document()->isModified();
}

bool CodeEditor::changedSinceParse()
{
    return m_changedSinceParse.load() != 0;
}

void CodeEditor::markParsed()
{
    m_changedSinceParse.store(0);
}

void CodeEditor::setAutoComplete(bool enable)
{
    m_autoComplete = enable;
}

void CodeEditor::setErrors(QList<LangError> errors)
{
//    m_errors = errors;

    // TODO check if errors have changed to avoid having to do all this below unnecessarily
    foreach(ErrorMarker *marker, m_errorMarkers) {
        delete marker;
    }
    m_errorMarkers.clear();

    foreach(LangError error, errors) {
        if (error.filename == filename().toStdString()) {
            m_errorMarkers.push_back(new ErrorMarker(m_lineNumberArea, error.lineNumber,
                                                     QString::fromStdString(error.getErrorText())));
        }
    }
    m_lineNumberArea->repaint();
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::showButton()
{
    QRect buttonRect = cursorRect();
    buttonRect.setX(buttonRect.x() + 40);
    buttonRect.setY(buttonRect.y() - 4);
    buttonRect.setWidth(12);
    buttonRect.setHeight(12);
    m_helperButton.setGeometry(buttonRect);
    m_helperButton.show();
}

void CodeEditor::hideButton()
{
//    m_helperButton.hide(); // TODO This currently hides the menu before it's shown...
    m_ButtonTimer.start();
//    m_autoCompleteMenu.hide();
}

void CodeEditor::helperButtonClicked()
{
    emit(requestAssistant(m_helperButton.pos()));
    m_helperButton.setChecked(false);
}

void CodeEditor::mouseIdleTimeout()
{
    if (this->hasFocus()) {
        QTextCursor cursor = cursorForPosition(QPoint(m_toolTip.x(),m_toolTip.y()));
        cursor.select(QTextCursor::WordUnderCursor);
        QString word = cursor.selectedText();
        if (!word.isEmpty()) {
            QString text = m_codeModel->getTooltipText(word);
            if (!text.isEmpty()) {
                int width = m_toolTip.fontMetrics().width(m_toolTip.text());
                QRect boundingRect  = m_toolTip.fontMetrics().boundingRect(m_toolTip.text());
                m_toolTip.setText(text);
                m_toolTip.setGeometry(m_toolTip.x() + this->lineNumberAreaWidth(), m_toolTip.y() + 10,
                                      width, boundingRect.height() + 50);
                m_toolTip.show();
            }
        } else {
            m_toolTip.hide();
        }
    }
}

void CodeEditor::insertAutoComplete()
{
    QString text = static_cast<QAction *>(sender())->data().toString();
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    setTextCursor(cursor);
    insertPlainText(text);
}

void CodeEditor::updateAutoCompleteMenu(QString currentWord)
{
    m_autoCompleteMenu.clear();
    QAction *activeAction = nullptr;
    if (currentWord[0].toUpper() == currentWord[0]) {
        QStringList functions = m_codeModel->getFunctions();
        foreach(QString functionName, functions) {
            if (functionName.left(currentWord.size()) == currentWord) {
                QString syntaxText = m_codeModel->getFunctionSyntax(functionName);
                QAction *syntaxAction = m_autoCompleteMenu.addAction(functionName, this, SLOT(insertAutoComplete()));
                syntaxAction->setData(syntaxText);
                if (!activeAction) { activeAction = syntaxAction; }
            }
        }
    } else if (currentWord[0].toLower() == currentWord[0]) {
        QStringList types = m_codeModel->getTypes();
        foreach(QString typeName, types) {
            if (typeName.left(currentWord.size()) == currentWord) {
                QString syntaxText = m_codeModel->getTypeSyntax(typeName);
                QAction *syntaxAction = m_autoCompleteMenu.addAction(typeName, this, SLOT(insertAutoComplete()));
                syntaxAction->setData(syntaxText);
                if (!activeAction) { activeAction = syntaxAction; }
            }
        }
    }
    if (activeAction) {
        m_autoCompleteMenu.setActiveAction(activeAction);
    }
//            m_autoCompleteMenu.setGeometry(20, 20, 50, 100);
}

void CodeEditor::markChanged(bool changed)
{
    document()->setModified(changed);
    m_changedSinceParse.store(1);
}

bool CodeEditor::eventFilter(QObject *obj, QEvent *event)
{
    // This function responds to events passed on from active auto complete menu
    if (m_autoComplete
            && obj == &m_autoCompleteMenu
            && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//        qDebug("Ate key press %d", keyEvent->key());
        QRegExp regex("\\w+");
        if (regex.indexIn(keyEvent->text()) >= 0) {
            this->insertPlainText(keyEvent->text());
            QTextCursor cursor = textCursor();
            cursor.select(QTextCursor::WordUnderCursor);
            QString currentWord = cursor.selectedText();
            updateAutoCompleteMenu(currentWord);
            return true;
        } else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Tab) {
            QAction *activeAction = this->m_autoCompleteMenu.activeAction();
            if (activeAction) {
                activeAction->trigger();
            }
            m_autoCompleteMenu.hide();
        } else {
            m_autoCompleteMenu.hide();
            return QObject::eventFilter(obj, event);
        }
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

QString CodeEditor::filename() const
{
    return m_filename;
}

void CodeEditor::setFilename(const QString &filename)
{
    m_filename = filename;
}

void CodeEditor::find(QString query)
{
    QTextCursor cursor = textCursor();
    if (query == "") {
        cursor.select(QTextCursor::WordUnderCursor);
        query = cursor.selectedText();
    }
    find(query);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    QPlainTextEdit::resizeEvent(e);
}

void CodeEditor::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        QTextCursor cursor = textCursor();
        QString previousText = toPlainText().left(cursor.position());
        int scopeLevel = previousText.count("{") + previousText.count("[");
        scopeLevel -= previousText.count("}") + previousText.count("]");
        for (int i = 0; i < scopeLevel; ++i) {
            if (m_IndentTabs) {
                cursor.insertText("\t");
            } else {
                cursor.insertText("    ");
            }
        }
        setTextCursor(cursor);
    } else if (m_autoComplete && event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z) {
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        QString currentWord = cursor.selectedText();
        if (currentWord.size() > 2) {
            QRect cursorRectValue = cursorRect(cursor);
            updateAutoCompleteMenu(currentWord);
            QPoint p = QPoint(cursorRectValue.x() + cursorRectValue.width(),
                              cursorRectValue.y() + cursorRectValue.height());
            QPoint globalPoint =  this->mapToGlobal(p);
            m_autoCompleteMenu.move(globalPoint);
            if (m_autoCompleteMenu.actions().size() > 0) {
                m_autoCompleteMenu.show();
            }
//            m_autoCompleteMenu.show();
        }
//        setFocus();
    } else {
        m_autoCompleteMenu.hide();
        setFocus();
    }
    hideButton();
    QPlainTextEdit::keyReleaseEvent(event);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event)
{
//    m_toolTip.setGeometry(event->x(), event->y(), 10, 10);
    m_toolTip.hide();
    m_mouseIdleTimer.start();
    QPlainTextEdit::mouseMoveEvent(event);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();
    foreach(ErrorMarker *marker, m_errorMarkers) {
        marker->hide();
    }
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);

            foreach(ErrorMarker *marker, m_errorMarkers) {
                if (marker->getLineNumber() == blockNumber + 1) {
                    marker->setGeometry(0, top, fontMetrics().width("9"), fontMetrics().height());
                    marker->show();
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::setToolTipText(QString text)
{
    m_toolTip.setText(text);
}
