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

#include "codeeditor.h"

#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>

#include "linenumberarea.h"

CodeEditor::CodeEditor(QWidget *parent, CodeModel *codeModel)
    : QPlainTextEdit(parent), m_codeModel(codeModel), m_autoCompleteMenu(this),
      m_IndentTabs(false), m_helperButton(this), m_toolTip((QWidget *)this) {
  setMouseTracking(true);
  m_lineNumberArea = new LineNumberArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this,
          SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(highlightCurrentLine()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
  m_ButtonTimer.setInterval(1400);
  m_ButtonTimer.setSingleShot(true);
  m_mouseIdleTimer.setInterval(900);
  m_mouseIdleTimer.setSingleShot(true);

  m_helperButton.hide();
  m_helperButton.setText("+");

  // TODO rethink helper button or remove
  //  connect(&m_ButtonTimer, SIGNAL(timeout()), this, SLOT(showButton()));
  connect(&m_mouseIdleTimer, SIGNAL(timeout()), this, SLOT(mouseIdleTimeout()));
  connect(&m_helperButton, SIGNAL(pressed()), this,
          SLOT(helperButtonClicked()));
  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(hideButton()));

  m_toolTip.hide();
  setContextMenuPolicy(Qt::CustomContextMenu);
}

int CodeEditor::lineNumberAreaWidth() {
  //    int digits = 1;
  //    int max = qMax(1, blockCount());
  //    while (max >= 10) {
  //        max /= 10;
  //        ++digits;
  //    }

  int digits = 4;
  int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
}

bool CodeEditor::isChanged() { return document()->isModified(); }

bool CodeEditor::changedSinceParse() {
  return m_changedSinceParse.load() != 0;
//  return m_changedSinceParse.loadRelaxed() != 0;
}

void CodeEditor::markParsed() {
  m_changedSinceParse.storeRelease(0);
//  m_changedSinceParse.storeRelaxed(0);
}

void CodeEditor::setAutoComplete(bool enable) { m_autoComplete = enable; }

void CodeEditor::setErrors(QList<LangError> errors) {
  QList<LangError> filteredErrors;
  for (auto error : errors) {
    if (QString::fromStdString(error.filename).endsWith(m_filename)) {
      filteredErrors.push_back(error);
    }
  }
  m_lineNumberArea->setErrors(filteredErrors);
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    m_lineNumberArea->scroll(0, dy);
  else
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(),
                             rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void CodeEditor::showButton() {
  QRect buttonRect = cursorRect();
  buttonRect.setX(buttonRect.x() + 40);
  buttonRect.setY(buttonRect.y() - 4);
  buttonRect.setWidth(12);
  buttonRect.setHeight(12);
  m_helperButton.setGeometry(buttonRect);
  m_helperButton.show();
}

void CodeEditor::hideButton() {
  //    m_helperButton.hide(); // TODO This currently hides the menu before it's
  //    shown...
  m_ButtonTimer.start();
  //    m_autoCompleteMenu.hide();
}

void CodeEditor::helperButtonClicked() {
  emit(requestAssistant(m_helperButton.pos()));
  m_helperButton.setChecked(false);
}

void CodeEditor::mouseIdleTimeout() {
  if (this->hasFocus()) {
    QTextCursor cursor =
        cursorForPosition(QPoint(m_toolTip.x(), m_toolTip.y()));
    cursor.select(QTextCursor::WordUnderCursor);
    QString word = cursor.selectedText();
    if (!word.isEmpty()) {
      QString text = m_codeModel->getTooltipText(word);
      if (!text.isEmpty()) {
        int width = m_toolTip.fontMetrics().horizontalAdvance(m_toolTip.text());
        QRect boundingRect =
            m_toolTip.fontMetrics().boundingRect(m_toolTip.text());
        m_toolTip.setText(text);
        m_toolTip.setGeometry(m_toolTip.x() + this->lineNumberAreaWidth(),
                              m_toolTip.y() + 10, width,
                              boundingRect.height() + 50);
        m_toolTip.show();
      }
    } else {
      m_toolTip.hide();
    }
  }
}

void CodeEditor::insertAutoComplete() {
  QString text = static_cast<QAction *>(sender())->data().toString();
  QTextCursor cursor = textCursor();
  cursor.select(QTextCursor::WordUnderCursor);
  setTextCursor(cursor);
  insertPlainText(text);
  qDebug() << "Insert " << text;
}

void CodeEditor::updateAutoCompleteMenu(QString currentWord) {
  auto currentActiveAction = m_autoCompleteMenu.activeAction();
  QString currentActive;
  if (currentActiveAction) {
    currentActive = currentActiveAction->text();
  }
  m_autoCompleteMenu.clear();
  QAction *activeAction = nullptr;
  if (m_currentContext == UseStatementSystem) {
    auto availableSystems = StrideSystem::listAvailableSystems(
        m_codeModel->getSystem()->getStrideRoot());
    for (auto systemName : availableSystems) {
      QAction *syntaxAction = m_autoCompleteMenu.addAction(
          QString::fromStdString(systemName), this, SLOT(insertAutoComplete()));
      syntaxAction->setData(QString::fromStdString(systemName) +
                            " version 1.0");
    }
  } else if (m_currentContext == UseStatementVersion) {

  } else if (m_currentContext == ImportStatement) {

    auto availableImports = m_codeModel->getSystem()->listAvailableImports();
    for (auto systemName : availableImports) {
      QAction *syntaxAction = m_autoCompleteMenu.addAction(
          QString::fromStdString(systemName), this, SLOT(insertAutoComplete()));
      syntaxAction->setData(QString::fromStdString(systemName));
    }
  } else {

    if (currentWord[0].toUpper() == currentWord[0]) {
      QStringList functions = m_codeModel->getFunctions();
      foreach (QString functionName, functions) {
        if (functionName.left(currentWord.size()) == currentWord) {
          QString syntaxText = m_codeModel->getFunctionSyntax(functionName);
          QAction *syntaxAction = m_autoCompleteMenu.addAction(
              functionName, this, SLOT(insertAutoComplete()));
          syntaxAction->setData(syntaxText);
          if (currentActive == functionName) {
            activeAction = syntaxAction;
          }
        }
      }
    } else if (currentWord[0].toLower() == currentWord[0]) {
      QStringList types = m_codeModel->getTypes();
      foreach (QString typeName, types) {
        if (typeName.left(currentWord.size()) == currentWord) {
          QString syntaxText = m_codeModel->getTypeSyntax(typeName);
          QAction *syntaxAction = m_autoCompleteMenu.addAction(
              typeName, this, SLOT(insertAutoComplete()));
          syntaxAction->setData(syntaxText);
          if (currentActive == typeName) {
            activeAction = syntaxAction;
          }
        }
      }
    }
  }
  if (activeAction) {
    m_autoCompleteMenu.setActiveAction(activeAction);
  } else {
  }
  //            m_autoCompleteMenu.setGeometry(20, 20, 50, 100);
}

void CodeEditor::markChanged(bool changed) {
  document()->setModified(changed);
//  m_changedSinceParse.storeRelaxed(1);
  m_changedSinceParse.storeRelease(1);
}

bool CodeEditor::eventFilter(QObject *obj, QEvent *event) {
  // This function responds to events passed on from active auto complete menu
  if (m_autoComplete && obj == &m_autoCompleteMenu &&
      event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    //        qDebug("Ate key press %d", keyEvent->key());
    QRegExp regex("\\w+");
    if (regex.indexIn(keyEvent->text()) >= 0) {
      this->event(event);
      QTextCursor cursor = textCursor();
      cursor.select(QTextCursor::WordUnderCursor);
      QString currentWord = cursor.selectedText();
      updateAutoCompleteMenu(currentWord);
      return true;
    } else if (keyEvent->key() == Qt::Key_Backspace ||
               keyEvent->key() == Qt::Key_Delete ||
               keyEvent->key() == Qt::Key_Left ||
               keyEvent->key() == Qt::Key_Right) {
      this->event(event);
      QTextCursor cursor = textCursor();
      cursor.select(QTextCursor::WordUnderCursor);
      QString currentWord = cursor.selectedText();
      updateAutoCompleteMenu(currentWord);
      return true;
    } else if (keyEvent->key() == Qt::Key_Return ||
               keyEvent->key() == Qt::Key_Tab) {
      QAction *activeAction = this->m_autoCompleteMenu.activeAction();
      if (activeAction) {
        activeAction->trigger();
      }
      m_autoCompleteMenu.hide();
      return true;
    } else if (keyEvent->key() == Qt::Key_Up ||
               keyEvent->key() == Qt::Key_Down) {
      return QObject::eventFilter(obj, event);
    } else {
      m_autoCompleteMenu.hide();
      return QObject::eventFilter(obj, event);
    }
  } else {
    // standard event processing
    return QObject::eventFilter(obj, event);
  }
}

QString CodeEditor::filename() const { return m_filename; }

void CodeEditor::setFilename(const QString &filename) { m_filename = filename; }

void CodeEditor::find(QString query) {
  QTextCursor cursor = textCursor();
  if (query == "") {
    cursor.select(QTextCursor::WordUnderCursor);
    query = cursor.selectedText();
  }
  QPlainTextEdit::find(query);
}

void CodeEditor::gotoLine(int line) {
  QTextCursor newCursor(this->document()->findBlockByNumber(
      line - 1)); // ln-1 because line number starts from 0
  this->setTextCursor(newCursor);
  qDebug() << verticalScrollBar()->value() << " "
           << verticalScrollBar()->maximum();
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
  QPlainTextEdit::resizeEvent(e);
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
  auto currentPosition = textCursor().position();
  auto lastStartOfScope =
      document()
          ->find("{", currentPosition, QTextDocument::FindBackward)
          .position();
  auto lastEndOfScope =
      document()
          ->find("}", currentPosition, QTextDocument::FindBackward)
          .position();

  auto lastStartOfArgs =
      document()
          ->find("(", currentPosition, QTextDocument::FindBackward)
          .position();
  auto lastEndOfArgs =
      document()
          ->find(")", currentPosition, QTextDocument::FindBackward)
          .position();

  // TODO determine if we are in streams.

  if (lastStartOfArgs > lastEndOfArgs) {

    m_currentContext = FunctionProperties;
    // In function call
    // TODO mark FunctionScope
  } else {
    if (lastStartOfScope <= lastEndOfScope) {
      // We are in the root scope
      auto cursor = textCursor();
      cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

      auto lineStartText = cursor.selectedText().trimmed();
      if (lineStartText.startsWith("import")) {
        m_currentContext = ImportStatement;
      } else if (lineStartText.startsWith("use")) {
        m_currentContext = UseStatementSystem;
        // TODO determine if we are in the version part
      } else {
        m_currentContext = RootScope;
      }
    }
  }

  // tab insertion gets processed between this function and keyReleaseEvent,
  // so we need to process changing tab to space here
  if (event->key() == Qt::Key_Tab && !m_IndentTabs) {
    QTextCursor cursor = textCursor();
    cursor.insertText("    ");
    setTextCursor(cursor);
    return;
  } else if (event->key() == Qt::Key_Return) {
    QTextCursor cursor = textCursor();
    QString previousText = toPlainText().left(cursor.position());
    int scopeLevel = previousText.count("{") + previousText.count("[");
    scopeLevel -= previousText.count("}") + previousText.count("]");
    cursor.insertText("\n");
    for (int i = 0; i < scopeLevel; ++i) {
      if (m_IndentTabs) {
        cursor.insertText("t");
      } else {
        cursor.insertText("    ");
      }
    }
    setTextCursor(cursor);
    return;
  } else if (event->key() == '}' || event->key() == ']') {
    QTextCursor cursor = textCursor();

    QString previousText = toPlainText().left(cursor.position());
    int scopeLevel = previousText.count("{") + previousText.count("[");
    scopeLevel -= previousText.count("}") + previousText.count("]");

    cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    if (cursor.selectedText().trimmed().size() == 0) {
      for (int i = 0; i < scopeLevel - 1; ++i) {
        if (m_IndentTabs) {
          cursor.insertText("\t");
        } else {
          cursor.insertText("    ");
        }
      }
      setTextCursor(cursor);
    }
  }
  QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::keyReleaseEvent(QKeyEvent *event) {
  if (m_autoComplete && event->key() >= Qt::Key_A &&
      event->key() <= Qt::Key_Z && event->modifiers() == Qt::NoModifier) {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    QString currentWord = cursor.selectedText();
    if (currentWord.size() > 2 || (m_currentContext == ImportStatement ||
                                   m_currentContext == UseStatementSystem ||
                                   m_currentContext == UseStatementVersion)) {
      QRect cursorRectValue = cursorRect(cursor);
      updateAutoCompleteMenu(currentWord);
      QPoint p = QPoint(cursorRectValue.x() + cursorRectValue.width(),
                        cursorRectValue.y() + cursorRectValue.height());
      QPoint globalPoint = this->mapToGlobal(p);
      m_autoCompleteMenu.move(globalPoint);
      if (m_autoCompleteMenu.actions().size() > 0) {
        if (m_autoCompleteMenu.activeAction() == nullptr) {
          m_autoCompleteMenu.setActiveAction(
              m_autoCompleteMenu.actions().at(0));
        }
        m_autoCompleteMenu.show();
      }
    }
    //        setFocus();
  } else {
    m_autoCompleteMenu.hide();
    setFocus();
  }
  hideButton();
  QPlainTextEdit::keyReleaseEvent(event);
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event) {
  m_toolTip.setGeometry(event->x(), event->y(), 10, 10);
  m_toolTip.hide();
  m_mouseIdleTimer.start();
  QPlainTextEdit::mouseMoveEvent(event);
}

void CodeEditor::highlightCurrentLine() {
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

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);
  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  {
    std::unique_lock<std::mutex> lk(m_lineNumberArea->m_markerLock);
    for (auto marker : m_lineNumberArea->m_errorMarkers) {
      if (marker->getLineNumber() < blockNumber) {
        marker->hide();
      }
    }
  }
  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(0, top, m_lineNumberArea->width(),
                       fontMetrics().height(), Qt::AlignRight, number);
      {
        std::unique_lock<std::mutex> lk(m_lineNumberArea->m_markerLock);
        for (auto marker : m_lineNumberArea->m_errorMarkers) {
          if (marker->getLineNumber() == blockNumber + 1) {
            marker->setGeometry(0, top, fontMetrics().horizontalAdvance("9"),
                                fontMetrics().height());
            marker->show();
          }
        }
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
  {
    std::unique_lock<std::mutex> lk(m_lineNumberArea->m_markerLock);
    for (auto marker : m_lineNumberArea->m_errorMarkers) {
      if (marker->getLineNumber() > blockNumber + 1) {
        marker->hide();
      }
    }
  }
}

void CodeEditor::setToolTipText(QString text) { m_toolTip.setText(text); }
