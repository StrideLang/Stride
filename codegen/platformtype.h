#ifndef PLATFORMTYPE_H
#define PLATFORMTYPE_H

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

class Property {
public:
    typedef enum {
        PropertyAccess,
        Stream_in,
        Stream_out,
        None
    } AccessType;

    QString name;
    QStringList types; // For types
    QStringList in_types; // in/out for functions
    QStringList out_types;
    QVariant defaultValue;
    bool required;
    int maxconnections;
    AccessType access;
};

class PlatformType
{
public:
    PlatformType(QString name, QList<Property> &properties);
    ~PlatformType();

    QString getName();
    bool hasPort(QString portName);
    bool isValidPortType(QString portName, QString type);

    QList<Property> ports() const;

private:
    QString m_name;
    QList<Property> m_ports;
    QStringList m_inherits;
};

#endif // PLATFORMTYPE_H
