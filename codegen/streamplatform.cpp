#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include "streamplatform.h"

StreamPlatform::StreamPlatform(QString platformPath) :
    m_platformRootPath(platformPath)
{
    initBasicTypes();
}

StreamPlatform::StreamPlatform(QString platformPath, QString platform, QString version) :
    m_platformRootPath(platformPath), m_platformName(platform), m_version(version)
{
    initBasicTypes();
    parsePlatformTypes();
    parsePlatformCommonTypes();
    parseCommonTypes();
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

QString StreamPlatform::getPlatformPath()
{
    return m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version;
}

void StreamPlatform::parsePlatformCommonTypes()
{
    QString platformFile = m_platformRootPath + QDir::separator() + "common"
            + QDir::separator() + "builtin_types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QString errorText = "Can't open common platform file: " +  f.fileName();
        m_errors << errorText;
        qDebug() << errorText;
    } else {
        Q_ASSERT(m_platformCommonTypes.isEmpty());
        parseTypesJson(f.readAll(), m_platformCommonTypes);
        f.close();
    }
}

void StreamPlatform::parseCommonTypes()
{
    QString platformFile = m_platformRootPath + QDir::separator() + m_platformName
            + QDir::separator() + "common/types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        QString errorText = "Can't open platform types: " +  f.fileName();
        m_errors << errorText;
        qDebug() << errorText;
    } else {
        Q_ASSERT(m_commonTypes.isEmpty());
        parseTypesJson(f.readAll(), m_commonTypes);
        f.close();
    }
}

void StreamPlatform::parseTypesJson(QString jsonText, QList<PlatformType> &m_types)
{
    QList<PlatformType> newTypes;
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonText.toLocal8Bit());
    QJsonObject obj = jdoc.object();
    QJsonValue types = obj.take("types");
    if(!types.isArray()) {
        QString errorText = "Error in JSON format.";
        m_errors << errorText;
        qDebug() << errorText;
    }
    QJsonArray typesArray = types.toArray();

    foreach(QJsonValue type, typesArray) {
        QJsonObject typeObj = type.toObject();
        QJsonValue val = typeObj.take("typeName");
        QString typeName = val.toString();
        QVariantList portsList = typeObj.take("ports").toVariant().toList();
        QList<Property> ports;
        foreach(QVariant port, portsList) {
            Property newPort;
            QMap<QString, QVariant> portMap = port.toMap();
            newPort.name = portMap.take("name").toString();
            if (newPort.name.isEmpty()) {
                QString errorText = "Empty name for port in: " + typeName;
                m_errors << errorText;
                qDebug() << errorText;
            }
            newPort.defaultValue = portMap.take("default");
            foreach(QVariant propertyType, portMap.take("types").toList()) {
                newPort.types << propertyType.toString();
            }
            newPort.maxconnections = portMap.take("maxconnections").toInt();
            QString accessString = portMap.take("access").toString();
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
            m_errors << errorText;
            qDebug() << errorText;
        }

        QJsonObject privateMap = typeObj.take("private").toObject();
        QVariantList inhertitsList = privateMap.take("inherits").toArray().toVariantList();
        foreach(QVariant member, inhertitsList) {
            ports.append(getPortsForType(member.toString()));
        }

        //.toVariantMap();
        PlatformType newType(typeName, ports);
        m_types.append(newType);
    }
}

QStringList StreamPlatform::getErrors()
{
    return m_errors;
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

void StreamPlatform::initBasicTypes()
{
    m_basicTypes << "int" << "real" << "list" << "string" << "stream";
}

