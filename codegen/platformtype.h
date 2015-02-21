#ifndef PLATFORMTYPE_H
#define PLATFORMTYPE_H

#include <QList>
#include <QHash>
#include <QString>

// TODO properties should have defaults
typedef QHash<QString, QList<QString> > Property;

class PlatformType
{
public:
    PlatformType(QString name, QList<Property> properties);
    ~PlatformType();

    QString getName();
    bool isValidProperty(QString propertyName);

private:
    QString m_name;
    QList<Property> m_properties;
};

#endif // PLATFORMTYPE_H
