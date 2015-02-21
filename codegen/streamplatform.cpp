#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "streamplatform.h"

StreamPlatform::StreamPlatform(QString platformPath)
{
    initBasicTypes();
}

StreamPlatform::StreamPlatform(QString platformPath, QString platform)
{
    initBasicTypes();
    QString platformFile = platformPath + QDir::separator() + "common"
            + QDir::separator() + "builtin_types.json";
    QFile f(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open common platform file: " << f.fileName();
    } else {
        parseJson(f.readAll());
        f.close();
    }

    platformFile = platformPath + QDir::separator() + platform
            + QDir::separator() + "common/types.json";
    f.setFileName(platformFile);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
       qDebug() << "Can't open platform types: " << f.fileName();
    } else {
        parseJson(f.readAll());
        f.close();
    }
}

StreamPlatform::~StreamPlatform()
{

}

void StreamPlatform::parseJson(QString jsonText)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonText.toLocal8Bit());
    QJsonObject obj = jdoc.object();
    qDebug() << obj;
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

