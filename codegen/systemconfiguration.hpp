#ifndef SYSTEMCONFIGURATION_HPP
#define SYSTEMCONFIGURATION_HPP

#include <QMap>
#include <QString>
#include <QVariant>

typedef QMap<QString, QVariant> ConfigMap;

class SystemConfiguration
{
public:
    SystemConfiguration();
    QMap<QString, ConfigMap> overrides;
    QMap<QString, ConfigMap> hardwareConfigurations;
    QMap<QString, ConfigMap> platformConfigurations;
};

#endif // SYSTEMCONFIGURATION_HPP
