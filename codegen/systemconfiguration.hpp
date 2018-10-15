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
    QMap<QString, ConfigMap> overrides; // Override existing values in Stride Code
    QMap<QString, ConfigMap> substitutions; // Substitute strings in generated code
    QMap<QString, ConfigMap> hardwareConfigurations; // Configure the hardware
    QMap<QString, ConfigMap> platformConfigurations; // Configure the platform
};

#endif // SYSTEMCONFIGURATION_HPP
