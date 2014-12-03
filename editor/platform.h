#ifndef PLATFORM_H
#define PLATFORM_H

#include <QString>

#include <string>
#include <vector>

struct TypeProperty {
    std::string name;
    std::vector<std::string> types;
    std::string defaultValue;
};

struct PlatformType {
    std::string typeName;
    std::vector<TypeProperty> properties;
    std::vector<TypeProperty> privateProperties;
};

class Platform
{
public:
    Platform(QString name);
    QString getPlatformName() {return m_platformName;}
    QString getToolchainPath() {return m_toolchainPath;}
    QString getPlatformPath() {return m_platformPath;}

    void setPlatformRoot(QString platformRoot) {m_platformPath = platformRoot + "/" + m_platformName;}
    void setToolchainPath(QString toolchainPath) {m_toolchainPath = toolchainPath;}

private:
    QString m_platformName;
    QString m_toolchainPath;
    QString m_platformPath;
};

#endif // PLATFORM_H
