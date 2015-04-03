#ifndef ERRORMARKER_H
#define ERRORMARKER_H

#include <QWidget>

class ErrorMarker : public QWidget
{
    Q_OBJECT
public:
    explicit ErrorMarker(QWidget *parent = 0, int lineNumber = -1, QString text = QString());
    ~ErrorMarker();

    int getLineNumber() {return m_lineNumber;}

signals:

public slots:

protected:
    virtual void paintEvent(QPaintEvent * event);
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);

private:
    int m_lineNumber;
    QString m_errorText;
    QWidget *m_text;
};

#endif // ERRORMARKER_H
