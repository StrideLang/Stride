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
    StreamPlatform(QString platformPath, QString platform);
    ~StreamPlatform();

//    void setHardwarePlatform(HardwarePlatform &platform);
    void parseBuiltinTypesJson(QString jsonText);
    QList<PlatformType> parseTypesJson(QString jsonText);

    bool isValidType(QString typeName);

private:
    void initBasicTypes();

    QString m_platformPath;

    QString m_platformName;
    QList<PlatformType> m_platformTypes;
    QList<PlatformFunction> m_platformFunctions;
    QList<PlatformObject> m_platformObjects;
    QStringList m_basicTypes;
    QList<PlatformType> m_builtinTypes;
};

#endif // STREAMPLATFORM_H
