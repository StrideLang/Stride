#include "platformtype.h"

PlatformType::PlatformType(QString name, QList<Property> properties) :
    m_name(name), m_properties(properties)
{
}

PlatformType::~PlatformType()
{

}

QString PlatformType::getName()
{
    return m_name;
}

bool PlatformType::isValidProperty(QString propertyName)
{
    foreach(Property property, m_properties) {
        if (property.contains(propertyName)) {
            return true;
        }
    }
    return false;
}

