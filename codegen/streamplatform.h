#ifndef STREAMPLATFORM_H
#define STREAMPLATFORM_H

#include <QList>
#include <QString>
#include <QStringList>

#include "platformfunction.h"
#include "platformtype.h"
#include "platformobject.h"

class StreamPlatform
{
public:
    StreamPlatform(QString platformPath);
    StreamPlatform(QString platformPath, QString platform, QString version);
    ~StreamPlatform();

//    void setHardwarePlatform(HardwarePlatform &platform);
    void parseTypesJson(QString jsonText, QList<PlatformType> &types);
    void parseFunctionsJson(QString jsonText, QList<PlatformFunction> &functions);
    void parseObjectsJson(QString jsonText, QList<PlatformObject> &objects);

    QStringList getErrors();
    QStringList getWarnings();
    QStringList getPlatformTypes();
    QStringList getFunctions();
    QList<PlatformObject> getBuiltinObjects();

    bool isValidType(QString typeName);
    bool typeHasPort(QString typeName, QString propertyName);
    bool isValidPortType(QString typeName, QString propertyName, QString propType);

    QString getPlatformPath(); // Path for the specific platform

private:
    void initBasicTypes();
    void parsePlatformCommonTypes();
    void parseCommonTypes();
    void parsePlatformTypes();
    void parsePlatformFunction();
    void parsePlatformObjects();

    QList<Property> getPortsForType(QString typeName);
    QList<Property> getPortsForFunction(QString typeName);

    QString m_platformRootPath;
    QString m_platformName;
    QString m_version;

    QStringList m_errors;
    QStringList m_warnings;

    QList<PlatformType> m_commonTypes;
    QList<PlatformType> m_platformCommonTypes;
    QList<PlatformType> m_platformTypes;
    QList<PlatformFunction> m_platformFunctions;
    QList<PlatformObject> m_platformObjects;
    QStringList m_basicTypes;
};

#endif // STREAMPLATFORM_H
