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

class StridePlatform
{
public:
    StridePlatform() {}
    StridePlatform(QStringList platformPaths, QString platform, QString version,
                   QMap<QString, QString> importList);
    ~StridePlatform();

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

    QString getPlatformDomain();  // The platform's default domain

//    BlockNode *getFunction(QString functionName);
    QList<AST *> getBuiltinObjectsCopy();
    QList<AST *> getBuiltinObjects();

//    bool typeHasPort(QString typeName, QString propertyName);

    QString getPlatformPath(); // Path for the specific platform

private:
    QVector<AST *> getPortsForTypeBlock(BlockNode *block);
//    ListNode *getPortsForFunction(QString typeName);

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
    QMap<QString, QString> m_importList;
    StrideLibrary m_library;
};

#endif // STREAMPLATFORM_H
