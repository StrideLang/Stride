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
    QList<PlatformType> parseTypesJson(QString jsonText);

    bool isValidType(QString typeName);
    bool typeHasProperty(QString typeName, QString propertyName);
    bool isValidPropertyType(QString typeName, QString propertyName, QString propType);

private:
    void initBasicTypes();
    void parsePlatformCommonTypes();
    void parseCommonTypes();
    void parsePlatformTypes();

    QString m_platformPath;
    QString m_platformName;
    QString m_version;

    QList<PlatformType> m_commonTypes;
    QList<PlatformType> m_platformCommonTypes;
    QList<PlatformType> m_platformTypes;
    QList<PlatformFunction> m_platformFunctions;
    QList<PlatformObject> m_platformObjects;
    QStringList m_basicTypes;
};

#endif // STREAMPLATFORM_H
