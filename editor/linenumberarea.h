#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

#include "codeeditor.h"

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor);
    ~LineNumberArea();

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    CodeEditor *m_codeEditor;
};

#endif // LINENUMBERAREA_H
