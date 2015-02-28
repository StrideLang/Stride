#include "platformtype.h"

PlatformType::PlatformType(QString name, QList<Property> properties) :
    m_name(name), m_ports(properties)
{
}

PlatformType::~PlatformType()
{

}

QString PlatformType::getName()
{
    return m_name;
}

bool PlatformType::hasPort(QString portName)
{
    foreach(Property property, m_ports) {
        if (property.name == portName) {
            return true;
        }
    }
    return false;
}

bool PlatformType::isValidPortType(QString portName, QString type)
{
    foreach(Property property, m_ports) {
        if (property.name == portName && property.types.contains(type)) {
            return true;
        }
    }
    return false;
}

QList<Property> PlatformType::ports() const
{
    return m_ports;
}


