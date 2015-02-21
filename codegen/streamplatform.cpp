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

StreamPlatform::StreamPlatform(QString platformPath, QString platform) :
    m_platformPath(platformPath)
{
    initBasicTypes();
    QString platformFile = platformPath + QDir::separator() + "common"
            + QDir::separator() + "builtin_types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open common platform file: " << f.fileName();
    } else {
        parseBuiltinTypesJson(f.readAll());
        f.close();
    }

    platformFile = platformPath + QDir::separator() + platform
            + QDir::separator() + "common/types.json";
    f.setFileName(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open platform types: " << f.fileName();
    } else {
        parseBuiltinTypesJson(f.readAll());
        f.close();
    }
}

StreamPlatform::~StreamPlatform()
{

}

void StreamPlatform::parseBuiltinTypesJson(QString jsonText)
{
    m_builtinTypes = parseTypesJson(jsonText);
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
            QHash<QString, QStringList> propertyList;
            foreach(QVariant propertyType, propertiesMap.take("validTypes").toList()) {
                qDebug() << propertyType;
                newProp.validTypes << propertyType.toString();
            }
            properties.append(newProp);
        }
        PlatformType newType(name, properties);
        newTypes.append(newType);
    }
    return newTypes;
}

bool StreamPlatform::isValidType(QString typeName)
{
    if (m_basicTypes.contains(typeName)) {
        return true;
    }
    foreach(PlatformType type, m_builtinTypes) {
        if (type.getName() == typeName) {
            return true;
        }
    }
    foreach(PlatformType type, m_platformTypes) {
        if (type.getName() == typeName) {
            return true;
        }
    }
    return false;
}

void StreamPlatform::initBasicTypes()
{
    m_basicTypes << "int" << "real" << "list" << "string" << "stream";
}

