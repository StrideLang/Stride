#include "linenumberarea.h"

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor) {
    m_codeEditor = editor;
}

LineNumberArea::~LineNumberArea() {

}

QSize LineNumberArea::sizeHint() const {
    return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    m_codeEditor->lineNumberAreaPaintEvent(event);
}
