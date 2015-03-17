#ifndef PLATFORMOBJECT_H
#define PLATFORMOBJECT_H

#include <QString>

class PlatformObject
{
public:
    PlatformObject(QString &name);
    ~PlatformObject();

    QString getName();

private:
    QString m_name;
};

#endif // PLATFORMOBJECT_H
