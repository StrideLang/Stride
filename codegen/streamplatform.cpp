#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include "streamplatform.h"
#include "baseproject.h"

StreamPlatform::StreamPlatform(QString platformPath) :
    m_platformRootPath(platformPath)
{
}

StreamPlatform::StreamPlatform(QString platformPath, QString platform, QString version) :
    m_platformRootPath(platformPath), m_platformName(platform), m_version(version),
    m_pluginLibrary(NULL)
{
    // FIXME validate platform existence

    QString pluginPath = m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version + QDir::separator() + "plugins";
    pluginPath = "/home/andres/Documents/src/XMOS/Odo/StreamStack/plugins";
    if (QFile::exists(pluginPath)) {
        // FIXME move loading somewhere else where it's done less often
        QStringList pluginFiles = QDir(pluginPath).entryList(QDir::Files | QDir::NoDotAndDotDot);
        foreach (QString file, pluginFiles) {
//            qDebug() << file;
            QLibrary pluginLibrary(pluginPath + QDir::separator() + file);
            if (!pluginLibrary.load()) {
                qDebug() << pluginLibrary.errorString();
                continue;
            }
            typedef BaseProject* (*create_object)(const char *projectDir, StreamPlatform platform, const char *xmosToolchainRoot);
            typedef void (*platform_name)(char *projectDir);
            typedef double (*platform_version)();

            create_object create = (create_object) pluginLibrary.resolve("create_object");
            platform_name get_name = (platform_name) pluginLibrary.resolve("platform_name");
            platform_version get_version = (platform_version) pluginLibrary.resolve("platform_version");
            if (create && get_name && get_version) {
                char name[32];
                get_name(name);
                double libversion = get_version();
//                qDebug() << "Loaded platform " << name << " version " << QString::number(libversion, 'f', 1);
                if (platform == QString(name) && (QString::number(libversion, 'f', 1) == version or version == "-1.0")) {
                    qDebug() << "Using Platform " << name << " version " << QString::number(libversion, 'f', 1);
                    QString projectDir = "/home/andres";
                    QString xmosRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";
                    BaseProject *m_project = create(projectDir.toLocal8Bit(), *this, xmosRoot.toLocal8Bit());
                }
            }

        }
//        m_type = PluginPlatform;
//        m_pluginLibrary = new QLibrary(m_platformRootPath + QDir::separator() + m_platformName
//                                    + QDir::separator() + m_version + QDir::separator() + "plugins/"
//                                    + m_platformName);
//        m_pluginLibrary->load();
    } else {

//        m_type = PythonPlatform;
    }
    parsePlatformTypes();
    parsePlatformFunctions();
    parsePlatformObjects();
}

StreamPlatform::~StreamPlatform()
{

}

void StreamPlatform::parsePlatformTypes()
{
    QString platformFile = m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version
            + QDir::separator() + "types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QString errorText = "Can't open platform file: " +  f.fileName();
        m_errors << errorText;
        qDebug() << errorText;
    } else {
        Q_ASSERT(m_platformTypes.isEmpty());
        parseTypesJson(f.readAll(), m_platformTypes);
        f.close();
    }
}

void StreamPlatform::parsePlatformFunctions()
{
    QString platformFile = m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version
            + QDir::separator() + "functions.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QString errorText = "Can't open platform file: " +  f.fileName();
        m_errors << errorText;
        qDebug() << errorText;
    } else {
        Q_ASSERT(m_platformFunctions.isEmpty());
        parseFunctionsJson(f.readAll(), m_platformFunctions);
        f.close();
    }
}

void StreamPlatform::parsePlatformObjects()
{
    QString platformFile = m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version
            + QDir::separator() + "objects.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QString errorText = "Can't open platform file for objects: " +  f.fileName();
        m_errors << errorText;
        qDebug() << errorText;
    } else {
        Q_ASSERT(m_platformObjects.isEmpty());
        parseObjectsJson(f.readAll(), m_platformObjects);
        f.close();
    }
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
    return m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version;
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

