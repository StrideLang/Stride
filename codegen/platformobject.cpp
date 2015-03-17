#include "platformobject.h"

PlatformObject::PlatformObject(QString &name) :
    m_name(name)
{

}

PlatformObject::~PlatformObject()
{

}

QString PlatformObject::getName()
{
    return m_name;
}

