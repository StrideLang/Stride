#ifndef PLATFORMTYPE_H
#define PLATFORMTYPE_H

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

class Property {
public:
    QString name;
    QStringList validTypes;
    QVariant defaultValue;
};

class PlatformType
{
public:
    PlatformType(QString name, QList<Property> properties);
    ~PlatformType();

    QString getName();
    bool hasProperty(QString propertyName);
    bool isValidPropertyType(QString propertyName, QString type);

private:
    QString m_name;
    QList<Property> m_properties;
};

#endif // PLATFORMTYPE_H
