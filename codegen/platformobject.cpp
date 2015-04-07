#include "platformobject.h"

PlatformObject::PlatformObject(QString &name, int size, QString &type, QVariantMap properties) :
    m_name(name), m_size(size), m_type(type), m_properties(properties)
{

}

PlatformObject::~PlatformObject()
{

}

QString PlatformObject::getName()
{
    return m_name;
}

QMap<QString, QVariant> PlatformObject::getProperties()
{
    return m_properties;
}

int PlatformObject::getSize()
{
    return m_size;
}

QString PlatformObject::getType()
{
    return m_type;
}


