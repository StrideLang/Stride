#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QList>
#include <QTimer>
#include <QPushButton>

#include "langerror.h"
#include "errormarker.h"

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool isChanged();

    void setErrors(QList<LangError> errors);

    QString filename() const;
    void setFilename(const QString &filename);

    void find(QString query = "");

public slots:
    void markChanged(bool changed = true);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void keyReleaseEvent(QKeyEvent * event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void showButton();
    void hideButton();
    void helperButtonClicked();

signals:

private:
    QWidget *m_lineNumberArea;
    QVector<ErrorMarker *> m_errorMarkers;
    QTimer m_ButtonTimer;

    // Properties
    QString m_filename;
    bool m_IndentTabs;
    QPushButton m_helperButton;
};

#endif // CODEEDITOR_H
