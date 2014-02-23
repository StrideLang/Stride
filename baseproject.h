#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include <QObject>
#include <QString>

class BaseProject : public QObject
{
    Q_OBJECT
public:
    BaseProject(QString dir);
    virtual ~BaseProject();

    virtual QString getType() {return m_projectType;}
    virtual void build() {}

protected:
    QString m_projectDir;
    QString m_projectType;

signals:
    void outputText(QString text);
    void errorText(QString text);
};

#endif // BASEPROJECT_H
