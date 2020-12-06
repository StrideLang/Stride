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

#include "linenumberarea.h"
#include "errormarker.h"

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor) {
  m_codeEditor = editor;
  for (size_t i = 0; i < numMarkers; i++) {
    m_errorMarkers.emplace_back(std::make_shared<ErrorMarker>(editor));
    m_errorMarkers.back()->hide();
    m_errorMarkers.back()->setLineNumber(-1);
  }
}

LineNumberArea::~LineNumberArea() {}

QSize LineNumberArea::sizeHint() const {
  return {m_codeEditor->lineNumberAreaWidth(), 0};
}

void LineNumberArea::setErrors(QList<LangError> errors) {
  std::unique_lock<std::mutex> lk(m_markerLock);
  for (auto marker : m_errorMarkers) {
    marker->hide();
    marker->setLineNumber(-1);
  }
  for (size_t i = 0; i < errors.size(); i++) {
    m_errorMarkers[i]->setLineNumber(errors[i].lineNumber);
    m_errorMarkers[i]->setErrorText(
        QString::fromStdString(errors[i].getErrorText()));
    m_errorMarkers[i]->show();
  }
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
  m_codeEditor->lineNumberAreaPaintEvent(event);
}
