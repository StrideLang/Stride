#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include "streamplatform.h"
#include "builder.h"
#include "pythonproject.h"

StreamPlatform::StreamPlatform(QString platformPath) :
    m_platformRootPath(platformPath)
{
}

StreamPlatform::StreamPlatform(QStringList platformPaths, QString platform, QString version) :
    m_platformName(platform), m_version(version), m_api(NullPlatform)
{
    QString objectsJson;
    QString typesJson;
    QString functionsJson;
//    QString platformPath = m_platformRootPath + QDir::separator() + m_platformName
//            + QDir::separator() + m_version + QDir::separator() + "plugins";
    platformPaths << "/home/andres/Documents/src/XMOS/Odo/StreamStack/plugins"; // TODO: un hard-code this
    foreach(QString path, platformPaths) {
        if (m_api != NullPlatform) {
            break; // Stop looking if platform has been found.
        }
        // FIXME move loading somewhere else where it's done less often (Or don't create a new StreamPlatform every time you parse)
        QString fullPath = path + QDir::separator() + m_platformName
                + QDir::separator() + m_version;
        if (QFile::exists(fullPath)) {
            // First try to find library platforms
            QStringList pluginFiles = QDir(fullPath).entryList(QDir::Files | QDir::NoDotAndDotDot);
            foreach (QString file, pluginFiles) {
                if (QLibrary::isLibrary(file)) {
                    QLibrary pluginLibrary(fullPath + QDir::separator() + file);
                    if (!pluginLibrary.load()) {
                        qDebug() << pluginLibrary.errorString();
                        continue;
                    }
                    create_object_t create = (create_object_t) pluginLibrary.resolve("create_object");
                    platform_name_t get_name = (platform_name_t) pluginLibrary.resolve("platform_name");
                    platform_version_t get_version = (platform_version_t) pluginLibrary.resolve("platform_version");
                    if (create && get_name && get_version) {
                        char name[32];
                        get_name(name);
                        double platformVersion = get_version();
                        //                qDebug() << "Loaded platform " << name << " version " << QString::number(libversion, 'f', 1);
                        if (m_platformName == QString(name) && (QString::number(platformVersion, 'f', 1) == m_version || m_version == "-1.0")) {
                            qDebug() << "Using Plugin Platform " << name << " version " << QString::number(platformVersion, 'f', 1);
                            m_platformPath = fullPath;
                            m_api = PluginPlatform;
                            m_pluginName = fullPath + QDir::separator() + file;
                            QString xmosToolChainRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";
                            Builder *builder = create(this, "", xmosToolChainRoot.toLocal8Bit() );

                            typesJson = builder->requestTypesJson();
                            functionsJson = builder->requestFunctionsJson();
                            objectsJson = builder->requestObjectsJson();
                            break;
                        }
                    }
                    pluginLibrary.unload();
                }
                if (m_api != NullPlatform) {
                    break; // Stop looking if platform has been found.
                }
                // Now try to find Python platform
                if (QFile::exists(fullPath)) {
                    typesJson = readFile(fullPath + QDir::separator() + "types.json");
                    functionsJson = readFile(fullPath + QDir::separator() + "functions.json");
                    objectsJson = readFile(fullPath + QDir::separator() + "objects.json");
                    if (m_errors.size() == 0) {
                        m_api = PythonPlatform;
                        m_platformPath = fullPath;
                    }
                }
            }
        }
    }

    if (m_api == NullPlatform) {
        qDebug() << "Platform not found!";
    }

    parseTypesJson(typesJson, m_platformTypes);
    parseFunctionsJson(functionsJson, m_platformFunctions);
    parseObjectsJson(objectsJson,m_platformObjects);
}

StreamPlatform::~StreamPlatform()
{

}

QList<Property> StreamPlatform::getPortsForType(QString typeName)
{
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName) {
            return type.ports();
        }
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        if (type.getName() == typeName) {
            return type.ports();
        }
    }
    foreach(PlatformType type, m_commonTypes) {
        if (type.getName() == typeName) {
            return type.ports();
        }
    }
    return QList<Property>();
}

QList<Property> StreamPlatform::getPortsForFunction(QString typeName)
{
    foreach(PlatformFunction function, m_platformFunctions) {
        if (function.getName() == typeName) {
            return function.ports();
        }
    }
    return QList<Property>();
}

QVariant StreamPlatform::getDefaultPortValueForType(QString typeName, QString portName)
{
    QList<Property> ports = getPortsForType(typeName);
    foreach(Property port, ports) {
        if (port.name == portName) {
            return port.defaultValue;
        }
    }
    return QVariant();
}

PlatformFunction StreamPlatform::getFunction(QString functionName)
{
    foreach(PlatformFunction func, m_platformFunctions) {
        if (func.getName() == functionName) {
            return func;
        }
    }
    QList<Property> properties;
    return PlatformFunction("", properties, 0, 0);
}

QString StreamPlatform::getPlatformPath()
{
    return m_platformPath;
}

void StreamPlatform::parseTypesJson(QString jsonText, QList<PlatformType> &types)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonText.toLocal8Bit());
    QJsonObject obj = jdoc.object();
    QJsonValue typesJson = obj.take("types");
    if(!typesJson.isArray()) {
        QString errorText = "Error in JSON format parsing types.";
        m_errors << errorText;
        qDebug() << errorText;
    }
    QJsonArray typesArray = typesJson.toArray();

    foreach(QJsonValue type, typesArray) {
        QJsonObject typeObj = type.toObject();
        QString typeName = typeObj.take("typeName").toString();
        QVariantMap portsMap = typeObj.take("ports").toObject().toVariantMap();
        QStringList portNames = portsMap.keys();
        QList<Property> ports;
        foreach(QString portName, portNames) {
            QVariantMap port = portsMap.value(portName).toMap();
            Property newPort;
            newPort.name = portName;
            newPort.defaultValue = port.take("default");
            newPort.required = port.take("required").toBool();
            foreach(QVariant propertyType, port.take("types").toList()) {
                newPort.types << propertyType.toString();
            }
            newPort.maxconnections = port.take("maxconnections").toInt();
            QString accessString = port.take("access").toString();
            if (accessString == "property") {
                newPort.access = Property::PropertyAccess;
            } else if (accessString == "stream_in") {
                newPort.access = Property::Stream_in;
            } else if (accessString == "stream_out") {
                newPort.access = Property::Stream_out;
            } else {
                newPort.access = Property::None;
                QString errorText = "Invalid port access: " + accessString + " for type " + typeName;
                m_errors << errorText;
                qDebug() << errorText;
            }
            ports.append(newPort);
        }
        if (isValidType(typeName)) {
            QString errorText = "Shadowing duplicated type: " + typeName;
            m_warnings << errorText;
            qDebug() << errorText;
        }

        QJsonObject privateMap = typeObj.take("privateports").toObject();
        QVariantList inhertitsList = privateMap.take("inherits").toArray().toVariantList();
        foreach(QVariant member, inhertitsList) {
            ports.append(getPortsForType(member.toString()));
        }

        //.toVariantMap();
        PlatformType newType(typeName, ports);
        types.append(newType);
    }
}

void StreamPlatform::parseFunctionsJson(QString jsonText, QList<PlatformFunction> &functions)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonText.toLocal8Bit());
    QJsonObject obj = jdoc.object();
    QJsonValue funcs = obj.take("functions");
    if(!funcs.isArray()) {
        QString errorText = "Error in JSON format parsing functions.";
        m_errors << errorText;
        qDebug() << errorText;
    }
    QJsonArray funcsArray = funcs.toArray();

    foreach(QJsonValue func, funcsArray) {
        QJsonObject funcObj = func.toObject();
        QJsonValue val = funcObj.take("functionName");
        QString funcName = val.toString();
        QJsonObject propList = funcObj.take("properties").toObject();
        QStringList keys = propList.keys();
        QList<Property> ports;
        foreach(QString key, keys) {
            Property newPort;
            QMap<QString, QVariant> portMap = propList.value(key).toVariant().toMap();
            newPort.name = key;
            if (newPort.name.isEmpty()) {
                QString errorText = "Empty name for port in function: " + funcName;
                m_errors << errorText;
                qDebug() << errorText;
            }
            newPort.defaultValue = portMap.take("default");
            foreach(QVariant propertyType, portMap.take("in_types").toList()) {
                newPort.in_types << propertyType.toString();
            }
            foreach(QVariant propertyType, portMap.take("out_types").toList()) {
                newPort.out_types << propertyType.toString();
            }
//            newPort.maxconnections = portMap.take("maxconnections").toInt();
//            QString accessString = portMap.take("access").toString();
//            if (accessString == "property") {
//                newPort.access = Property::PropertyAccess;
//            } else if (accessString == "stream_in") {
//                newPort.access = Property::Stream_in;
//            } else if (accessString == "stream_out") {
//                newPort.access = Property::Stream_out;
//            } else {
//                newPort.access = Property::None;
//                QString errorText = "Invalid port access: " + accessString + " for type " + funcName;
//                m_errors << errorText;
//                qDebug() << errorText;
//            }
            ports.append(newPort);
        }
        if (isValidType(funcName)) {
            QString errorText = "Shadowing duplicated type: " + funcName;
            m_errors << errorText;
            qDebug() << errorText;
        }

        QJsonObject privateMap = funcObj.take("private").toObject();
        QVariantList inhertitsList = privateMap.take("inherits").toArray().toVariantList();
        foreach(QVariant member, inhertitsList) {
            ports.append(getPortsForType(member.toString()));
        }

        //.toVariantMap();

        int numInputs = funcObj.take("num_inputs").toInt(0);
        int numOutputs = funcObj.take("num_outputs").toInt(0);
        PlatformFunction newType(funcName, ports, numInputs, numOutputs);
        functions.append(newType);
    }
}

void StreamPlatform::parseObjectsJson(QString jsonText, QList<PlatformObject> &objects)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonText.toLocal8Bit());
    QJsonObject obj = jdoc.object();
    QJsonValue objectsValue = obj.take("objects");
    if(!objectsValue.isArray()) {
        QString errorText = "Error in JSON format parsing objects.";
        m_errors << errorText;
        qDebug() << errorText;
    }
    QJsonArray objectsArray = objectsValue.toArray();

    foreach(QJsonValue objectsValues, objectsArray) {
        QJsonObject objectObj = objectsValues.toObject();
        QJsonValue val = objectObj.take("objectName");
        QString objectName = val.toString();
        val = objectObj.take("size");
        int size = val.toInt(-1);
        val = objectObj.take("type");
        QString type = val.toString();
        // The remaining keys in the json object are considered properties

        PlatformObject newObj(objectName, size, type, objectObj.toVariantMap());
        objects.append(newObj);
    }
}

QString StreamPlatform::readFile(QString fileName)
{
    QString text;
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QString errorText = "Error opening platform file: " +  f.fileName();
        m_errors << errorText;
        qDebug() << errorText;
    } else {
        text = f.readAll();
        f.close();
    }
    return text;
}

QStringList StreamPlatform::getErrors()
{
    return m_errors;
}

QStringList StreamPlatform::getWarnings()
{
    return m_warnings;
}

QStringList StreamPlatform::getPlatformTypeNames()
{
    QStringList typeNames;
    foreach(PlatformType type, m_platformTypes) {
        typeNames << type.getName();
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        typeNames << type.getName();
    }
    foreach(PlatformType type, m_commonTypes) {
        typeNames << type.getName();
    }
    return typeNames;
}

QStringList StreamPlatform::getFunctionNames()
{
    QStringList functionNames;
    foreach(PlatformFunction func, m_platformFunctions) {
        functionNames << func.getName();
    }
    return functionNames;

}

Builder *StreamPlatform::createBuilder(QString projectDir)
{
    Builder *builder = NULL;
    if (m_api == StreamPlatform::PythonPlatform) {
        QString pythonExec = "python";
        builder = new PythonProject(this, projectDir, pythonExec);
    } else if(m_api == StreamPlatform::PluginPlatform) {
        QString xmosRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";

        QLibrary pluginLibrary(m_pluginName);
        if (!pluginLibrary.load()) {
            qDebug() << pluginLibrary.errorString();
            return NULL;
        }
        create_object_t create = (create_object_t) pluginLibrary.resolve("create_object");
        if (create) {
            builder = create(this, projectDir.toLocal8Bit(), xmosRoot.toLocal8Bit());
            }
        pluginLibrary.unload();
    }
    if (!builder->isValid()) {
        delete builder;
        builder = NULL;
    }
    return builder;
}

QList<PlatformObject> StreamPlatform::getBuiltinObjects()
{
    return m_platformObjects;
}

bool StreamPlatform::isValidType(QString typeName)
{
//    if (m_basicTypes.contains(typeName)) {
//        return true;
//    }
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName) {
            return true;
        }
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        if (type.getName() == typeName) {
            return true;
        }
    }
    foreach(PlatformType type, m_commonTypes) {
        if (type.getName() == typeName) {
            return true;
        }
    }
    return false;
}

bool StreamPlatform::typeHasPort(QString typeName, QString propertyName)
{
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName && type.hasPort(propertyName)) {
            return true;
        }
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        if (type.getName() == typeName && type.hasPort(propertyName)) {
            return true;
        }
    }
    foreach(PlatformType type, m_commonTypes) {
        if (type.getName() == typeName && type.hasPort(propertyName)) {
            return true;
        }
    }
    return false;
}

bool StreamPlatform::isValidPortType(QString typeName, QString propertyName, QString propType)
{
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName && type.hasPort(propertyName)
                && type.isValidPortType(propertyName, propType)) {
            return true;
        }
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        if (type.getName() == typeName && type.hasPort(propertyName)
                && type.isValidPortType(propertyName, propType)) {
            return true;
        }
    }
    foreach(PlatformType type, m_commonTypes) {
        if (type.getName() == typeName && type.hasPort(propertyName)
                && type.isValidPortType(propertyName, propType)) {
            return true;
        }
    }
    return false;
}

