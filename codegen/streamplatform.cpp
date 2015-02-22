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
    m_platformPath(platformPath)
{
    initBasicTypes();
}

StreamPlatform::StreamPlatform(QString platformPath, QString platform, QString version) :
    m_platformPath(platformPath), m_platformName(platform), m_version(version)
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
    QString platformFile = m_platformPath + QDir::separator() + m_platformName
            + QDir::separator() + m_version
            + QDir::separator() + "types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open platform file: " << f.fileName();
    } else {
        m_platformTypes << parseTypesJson(f.readAll());
        f.close();
    }
}

void StreamPlatform::parsePlatformCommonTypes()
{
    QString platformFile = m_platformPath + QDir::separator() + "common"
            + QDir::separator() + "builtin_types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open common platform file: " << f.fileName();
    } else {
        m_platformTypes << parseTypesJson(f.readAll());
        f.close();
    }
}

void StreamPlatform::parseCommonTypes()
{
    QString platformFile = m_platformPath + QDir::separator() + m_platformName
            + QDir::separator() + "common/types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open platform types: " << f.fileName();
    } else {
        m_commonTypes = parseTypesJson(f.readAll());
        f.close();
    }
}

QList<PlatformType> StreamPlatform::parseTypesJson(QString jsonText)
{
    QList<PlatformType> newTypes;
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonText.toLocal8Bit());
    QJsonObject obj = jdoc.object();
    QJsonValue types = obj.take("types");
    if(!types.isArray()) {
        qDebug() << "Error in JSON format.";
        return newTypes;
    }
    QJsonArray typesArray = types.toArray();

    foreach(QJsonValue type, typesArray) {
        QJsonObject typeObj = type.toObject();
        QJsonValue val = typeObj.take("typeName");
        QString name = val.toString();
        QVariantList propsList = typeObj.take("properties").toVariant().toList();
        QList<Property> properties;
        foreach(QVariant property, propsList) {
            Property newProp;
            QMap<QString, QVariant> propertiesMap = property.toMap();
            newProp.name = propertiesMap.take("name").toString();
            if (newProp.name.isEmpty()) {
                qDebug() << "Warning, empty name for property in: " << name;
            }
            newProp.defaultValue = propertiesMap.take("default");
            foreach(QVariant propertyType, propertiesMap.take("validTypes").toList()) {
                newProp.validTypes << propertyType.toString();
            }
            properties.append(newProp);
        }
        if (isValidType(name)) {
            qDebug() << "warning: shadowing duplicated type: " << name;
        }
        PlatformType newType(name, properties);
        newTypes.append(newType);
    }
    return newTypes;
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

bool StreamPlatform::typeHasProperty(QString typeName, QString propertyName)
{
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName && type.hasProperty(propertyName)) {
            return true;
        }
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        if (type.getName() == typeName && type.hasProperty(propertyName)) {
            return true;
        }
    }
    foreach(PlatformType type, m_commonTypes) {
        if (type.getName() == typeName && type.hasProperty(propertyName)) {
            return true;
        }
    }
}

bool StreamPlatform::isValidPropertyType(QString typeName, QString propertyName, QString propType)
{
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName && type.hasProperty(propertyName)
                && type.isValidPropertyType(propertyName, propType)) {
            return true;
        }
    }
    foreach(PlatformType type, m_platformCommonTypes) {
        if (type.getName() == typeName && type.hasProperty(propertyName)
                && type.isValidPropertyType(propertyName, propType)) {
            return true;
        }
    }
    foreach(PlatformType type, m_commonTypes) {
        if (type.getName() == typeName && type.hasProperty(propertyName)
                && type.isValidPropertyType(propertyName, propType)) {
            return true;
        }
    }
}

void StreamPlatform::initBasicTypes()
{
    m_basicTypes << "int" << "real" << "list" << "string" << "stream";
}

