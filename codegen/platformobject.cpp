#include "platformobject.h"

PlatformObject::PlatformObject(QString &name, int size, QString &type, QVector<CodeGenData> &code) :
    m_name(name), m_size(size), m_type(type), m_code(code)
{

}

PlatformObject::~PlatformObject()
{

}

QString PlatformObject::getName()
{
    return m_name;
}

int PlatformObject::getSize()
{
    return m_size;
}

QString PlatformObject::getType()
{
    return m_type;
}

QVector<CodeGenData> PlatformObject::getCode()
{
    return m_code;
}

