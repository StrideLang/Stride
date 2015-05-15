#ifndef STREAMPLATFORM_H
#define STREAMPLATFORM_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QLibrary>

#include "platformfunction.h"
#include "platformtype.h"
#include "platformobject.h"
#include "baseproject.h"

class StreamPlatform
{
public:
    StreamPlatform(QString platformPath);
    StreamPlatform(QString platformPath, QString platform, QString version);
    ~StreamPlatform();

    typedef enum {
        PythonPlatform,
        PluginPlatform
    } PlatformAPI;

    QStringList getErrors();
    QStringList getWarnings();
    QStringList getPlatformTypeNames();
    QStringList getFunctionNames();
    PlatformAPI getAPI() {return m_type;}

    QList<Property> getPortsForType(QString typeName);
    QList<Property> getPortsForFunction(QString typeName);
    QVariant getDefaultPortValueForType(QString typeName, QString portName);

    PlatformFunction getFunction(QString functionName);
    QList<PlatformObject> getBuiltinObjects();

    bool isValidType(QString typeName);
    bool typeHasPort(QString typeName, QString propertyName);
    bool isValidPortType(QString typeName, QString propertyName, QString propType);

    QString getPlatformPath(); // Path for the specific platform

private:
    void parseTypesJson(QString jsonText, QList<PlatformType> &types);
    void parseFunctionsJson(QString jsonText, QList<PlatformFunction> &functions);
    void parseObjectsJson(QString jsonText, QList<PlatformObject> &objects);

    void parsePlatformTypes();
    void parsePlatformFunctions();
    void parsePlatformObjects();

    QString m_platformRootPath;
    QString m_platformName;
    QString m_version;

    QStringList m_errors;
    QStringList m_warnings;

    PlatformAPI m_type;
    QLibrary *m_pluginLibrary;
    BaseProject *m_project;

    QList<PlatformType> m_commonTypes;
    QList<PlatformType> m_platformCommonTypes;
    QList<PlatformType> m_platformTypes;
    QList<PlatformFunction> m_platformFunctions;
    QList<PlatformObject> m_platformObjects;
};

#endif // STREAMPLATFORM_H
