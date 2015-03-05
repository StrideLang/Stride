#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QList>

//FIXME move lang error out of codegen
#include "codegen.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setErrors(QList<LangError> errors);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *m_lineNumberArea;
    QList<LangError> m_errors;

signals:


};

#endif // CODEEDITOR_H
