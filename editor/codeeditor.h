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
#include "autocompletemenu.hpp"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent, CodeModel *codeModel);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool isChanged();

    void setAutoComplete(bool enable);

    void setErrors(QList<LangError> errors);
    void setToolTipText(QString text);

    QString filename() const;
    void setFilename(const QString &filename);

    void find(QString query = "");

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
    QVector<ErrorMarker *> m_errorMarkers;
    QTimer m_ButtonTimer;
    QTimer m_mouseIdleTimer;

    // Properties
    QString m_filename;
    bool m_IndentTabs;
    bool m_autoComplete;

    QPushButton m_helperButton;
    ToolTip m_toolTip;

signals:

};



#endif // CODEEDITOR_H
