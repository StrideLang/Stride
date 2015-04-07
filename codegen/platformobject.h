#ifndef PLATFORMOBJECT_H
#define PLATFORMOBJECT_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QVariant>


class PlatformObject
{
public:
    PlatformObject(QString &name, int size, QString &type, QVariantMap properties);
    ~PlatformObject();

    QString getName();
    QMap<QString, QVariant> getProperties();
    int getSize();
    QString getType();

private:
    QString m_name;
    int m_size;
    QString m_type;
    QVariantMap m_properties;
};

#endif // PLATFORMOBJECT_H
