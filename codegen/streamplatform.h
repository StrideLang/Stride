#ifndef STREAMPLATFORM_H
#define STREAMPLATFORM_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QLibrary>
#include <QMap>


#include "ast.h"
#include "propertynode.h"
#include "blocknode.h"
#include "strideparser.h"
#include "stridelibrary.hpp"

#include "builder.h"

class StreamPlatform
{
public:
    StreamPlatform(QStringList platformPaths, QString platform, QString version);
    ~StreamPlatform();

    typedef enum {
        PythonTools,
        PluginPlatform,
        NullPlatform
    } PlatformAPI;

    QStringList getErrors();
    QStringList getWarnings();
    QStringList getPlatformTypeNames();
    QStringList getFunctionNames();
    PlatformAPI getAPI() {return m_api;}
    Builder *createBuilder(QString projectDir);

    QVector<AST *> getPortsForType(QString typeName);
    QVector<AST *> getPortsForTypeBlock(BlockNode *block);

    ListNode *getPortsForFunction(QString typeName);

    BlockNode *getFunction(QString functionName);
    QList<AST *> getBuiltinObjects();

    bool isValidType(QString typeName);
    bool typeHasPort(QString typeName, QString propertyName);
    bool isValidPortType(QString typeName, QString propertyName, PortType propType);

    QString getPlatformPath(); // Path for the specific platform

private:

    QString readFile(QString fileName);

    QString m_platformRootPath;
    QString m_platformPath;
    QString m_platformName;
    QString m_version;
    QString m_pluginName;
    PlatformAPI m_api;

    QStringList m_errors;
    QStringList m_warnings;


    QStringList m_types;

    QMap<QString, AST *> m_platform;
    StrideLibrary m_library;
};

#endif // STREAMPLATFORM_H
