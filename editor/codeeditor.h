#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QList>
#include <QTimer>
#include <QPushButton>

#include "langerror.h"
#include "errormarker.h"
#include "tooltip.hpp"
#include "codemodel.hpp"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent, CodeModel *codeModel);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool isChanged();

    void setErrors(QList<LangError> errors);
    void setToolTipText(QString text);

    QString filename() const;
    void setFilename(const QString &filename);

    void find(QString query = "");

public slots:
    void markChanged(bool changed = true);

protected:
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

signals:

private:
    QWidget *m_lineNumberArea;
    CodeModel *m_codeModel;
    QVector<ErrorMarker *> m_errorMarkers;
    QTimer m_ButtonTimer;
    QTimer m_mouseIdleTimer;

    // Properties
    QString m_filename;
    bool m_IndentTabs;
    QPushButton m_helperButton;
    ToolTip m_toolTip;
};



#endif // CODEEDITOR_H
