#ifndef PLATFORMOBJECT_H
#define PLATFORMOBJECT_H

#include <QString>
#include <QMap>
#include <QVector>

class CodeGenData  {
    QString section;
    QString code;
};

class PlatformObject
{
public:
    PlatformObject(QString &name, int size, QString &type, QVector<CodeGenData> &code);
    ~PlatformObject();

    QString getName();
    int getSize();
    QString getType();
    QVector<CodeGenData> getCode();

private:
    QString m_name;
    int m_size;
    QString m_type;
    QVector<CodeGenData> m_code;
};

#endif // PLATFORMOBJECT_H
