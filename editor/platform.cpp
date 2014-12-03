#include "platform.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

#include <QtGlobal>
#include <QDebug>


Platform::Platform(QString name) :
    m_platformName(name)
{
    QString platformsDir = "/home/andres/Documents/src/XMOS/Odo/StreamStack/platforms/";
    QFile builtinFile(platformsDir + "common/builtin_types.json");
    if (!builtinFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening file";
    }
    QJsonParseError error;
    QJsonDocument builtinsDoc = QJsonDocument::fromJson(builtinFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() <<"Error parsing: " << error.errorString();
        qDebug() << "at: " << error.offset;
    }
    Q_ASSERT(builtinsDoc.toVariant().canConvert<QVariantMap>());
    Q_ASSERT(builtinsDoc.toVariant().toMap().contains("types"));
    Q_ASSERT(builtinsDoc.toVariant().toMap().value("types").canConvert<QVariantList>());

    QVariantList builtinTypes = builtinsDoc.toVariant().toMap().value("types").toList();

    std::vector<PlatformType> builtinTypes_stl;
    foreach(QVariant type, builtinTypes) {
        PlatformType type_stl;
        type_stl.typeName = type.toMap().value("typeName").toString().toStdString();
        foreach(QVariant property, type.toMap().value("properties").toList()) {
            TypeProperty property_stl;
            property_stl.name = property.toMap().value("name").toString().toStdString();
            property_stl.defaultValue = property.toMap().value("default").toString().toStdString();

            foreach(QVariant validType, property.toMap().value("validTypes").toList()) {
                property_stl.types.push_back(validType.toString().toStdString());
            }
            type_stl.properties.push_back(property_stl);
        }
        builtinTypes_stl.push_back(type_stl);
    }
}
