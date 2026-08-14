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

#include <QDebug>

#include <QMutexLocker>
#include <QRegularExpression>
#include <QTextDocument>

#include "languagehighlighter.h"

LanguageHighlighter::LanguageHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent) {
  setFormatPreset(0);
  m_keywords << "none"
             << "on"
             << "off"
             << "streamRate"
             << "use"
             << "version"
             << "import";
}

void LanguageHighlighter::highlightBlock(const QString &text) {
  QMutexLocker locker(&m_highlighterLock);

  QRegularExpression expression;
  QString pattern;
  int index;

  // Properties/ports
  pattern = "(([a-z][a-zA-Z0-9_]*)[\\s]*:)";
  expression.setPattern(pattern);
  index = text.indexOf(expression);
  while (index >= 0) {
    auto match = QRegularExpressionMatch{};
    index = text.lastIndexOf(expression, index + 1, &match);
    int length = match.captured(0).size();
    setFormat(index, length, m_formats["ports"]);
    index = text.indexOf(expression, index + length);
  }

  for (const QString &keyword : m_keywords) {
    pattern = QString("\\b%1\\b").arg(keyword);
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
      auto match = QRegularExpressionMatch{};
      index = text.lastIndexOf(expression, index + 1, &match);
      int length = match.captured(0).size();
      setFormat(index, length, m_formats["keywords"]);
      index = text.indexOf(expression, index + length);
    }
  }

  for (const QString &blockType : m_blockTypes) {
    pattern = QString("\\b%1\\b").arg(blockType);
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
      auto match = QRegularExpressionMatch{};
      index = text.lastIndexOf(expression, index + 1, &match);
      int length = match.captured(0).size();
      setFormat(index, length, m_formats["type"]);
      index = text.indexOf(expression, index + length);
    }
  }

  for (const QString &objectName : m_builtinNames) {
    pattern = QString("\\b%1\\b").arg(objectName);
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
      auto match = QRegularExpressionMatch{};
      index = text.lastIndexOf(expression, index + 1, &match);
      int length = match.captured(0).size();
      setFormat(index, length, m_formats["builtin"]);
      index = text.indexOf(expression, index + length);
    }
  }

  for (QString functionName : m_functionNames) {
    pattern = QString("\\b%1\\b").arg(functionName);
    expression.setPattern(pattern);
    index = text.indexOf(expression);
    while (index >= 0) {
      auto match = QRegularExpressionMatch{};
      index = text.lastIndexOf(expression, index + 1, &match);
      int length = match.captured(0).size();
      setFormat(index, length, m_formats["function"]);
      index = text.indexOf(expression, index + length);
    }
  }

  pattern = ">>";
  expression.setPattern(pattern);
  index = text.indexOf(expression);
  while (index >= 0) {
    auto match = QRegularExpressionMatch{};
    index = text.lastIndexOf(expression, index + 1, &match);
    int length = match.captured(0).size();
    setFormat(index, length, m_formats["streamOp"]);
    index = text.indexOf(expression, index + length);
  }

  pattern = "\\\"(\\.|[^\"])*\\\"";
  expression.setPattern(pattern);
  index = text.indexOf(expression);
  while (index >= 0) {
    auto match = QRegularExpressionMatch{};
    index = text.lastIndexOf(expression, index + 1, &match);
    int length = match.captured(0).size();
    setFormat(index, length, m_formats["strings"]);
    index = text.indexOf(expression, index + length);
  }

  pattern = "'(\\.|[^'])*'";
  expression.setPattern(pattern);
  index = text.indexOf(expression);
  while (index >= 0) {
    auto match = QRegularExpressionMatch{};
    index = text.lastIndexOf(expression, index + 1, &match);
    int length = match.captured(0).size();
    setFormat(index, length, m_formats["strings"]);
    index = text.indexOf(expression, index + length);
  }

  // Leave comments for last
  pattern = "\\#.*";
  expression.setPattern(pattern);
  index = text.indexOf(expression);
  while (index >= 0) {
    auto match = QRegularExpressionMatch{};
    index = text.lastIndexOf(expression, index + 1, &match);
    int length = match.captured(0).size();
    setFormat(index, length, m_formats["comments"]);
    index = text.indexOf(expression, index + length);
  }
}

QMap<QString, QTextCharFormat> LanguageHighlighter::formats() {
  QMutexLocker locker(&m_highlighterLock);
  return m_formats;
}

void LanguageHighlighter::setFormats(
    const QMap<QString, QTextCharFormat> &formats) {
  m_highlighterLock.lock();
  m_formats = formats;
  m_highlighterLock.unlock();
  rehighlight();
}

void LanguageHighlighter::setBlockTypes(QStringList blockTypes) {
  m_highlighterLock.lock();
  m_blockTypes = blockTypes;
  m_highlighterLock.unlock();
  rehighlight();
}

void LanguageHighlighter::setFunctions(QStringList functionNames) {
  m_highlighterLock.lock();
  m_functionNames = functionNames;
  m_highlighterLock.unlock();
  rehighlight();
}

void LanguageHighlighter::setBuiltinObjects(QStringList builtinNames) {
  m_highlighterLock.lock();
  m_builtinNames = builtinNames;
  m_highlighterLock.unlock();
  rehighlight();
}

void LanguageHighlighter::setFormatPreset(int index) {
  QTextCharFormat keywordFormat;
  QTextCharFormat commentsFormat;
  QTextCharFormat typeFormat;
  QTextCharFormat functionFormat;
  QTextCharFormat propertiesFormat;
  QTextCharFormat builtinFormat;
  QTextCharFormat stringFormat;
  QTextCharFormat streamOpFormat;

  if (index == 0) {
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(QColor(0xff9900));
    keywordFormat.setBackground(Qt::white);

    commentsFormat.setFontWeight(QFont::Normal);
    commentsFormat.setForeground(Qt::black);
    commentsFormat.setBackground(Qt::green);

    typeFormat.setFontWeight(QFont::Bold);
    typeFormat.setForeground(QColor(0xcc0000));
    typeFormat.setBackground(Qt::white);

    functionFormat.setFontWeight(QFont::Normal);
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::black);
    functionFormat.setBackground(Qt::white);

    propertiesFormat.setFontWeight(QFont::Normal);
    propertiesFormat.setForeground(Qt::blue);
    propertiesFormat.setBackground(Qt::white);

    builtinFormat.setFontWeight(QFont::Bold);
    builtinFormat.setForeground(QColor(0xcc0000));
    builtinFormat.setBackground(Qt::white);

    stringFormat.setFontWeight(QFont::Bold);
    stringFormat.setForeground(QColor(0x33CC00));
    stringFormat.setBackground(Qt::white);

    streamOpFormat.setFontWeight(99);
    streamOpFormat.setForeground(QColor(0xBBCCBB));
    streamOpFormat.setBackground(Qt::white);
  } else {
    return;
  }

  m_formats["keywords"] = keywordFormat;
  m_formats["comments"] = commentsFormat;
  m_formats["type"] = typeFormat;
  m_formats["function"] = functionFormat;
  m_formats["ports"] = propertiesFormat;
  m_formats["builtin"] = builtinFormat;
  m_formats["strings"] = stringFormat;
  m_formats["streamOp"] = streamOpFormat;

  emit currentHighlightingChanged(m_formats);
}
