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

bool PlatformType::hasProperty(QString propertyName)
{
    foreach(Property property, m_properties) {
        if (property.name == propertyName) {
            return true;
        }
    }
    return false;
}

bool PlatformType::isValidPropertyType(QString propertyName, QString type)
{
    foreach(Property property, m_properties) {
        if (property.name == propertyName && property.validTypes.contains(type)) {
            return true;
        }
    }

}

