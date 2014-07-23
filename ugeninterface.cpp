#include "ugeninterface.h"

#include <QXmlStreamReader>
#include <QDir>
#include <QFile>
#include <QDebug>

UgenInterface::UgenInterface(QObject *parent) :
    QObject(parent)
{
    QString ugenPath = "/home/andres/Documents/src/XMOS/Odo/OdoEdit/Ugens";
    QDir ugenDir(ugenPath);
    QStringList ugenFiles = ugenDir.entryList(QStringList() << "*.xml", QDir::Files);

    qDebug() << ugenFiles;

    foreach(QString file, ugenFiles) {
        QFile ugenFile(ugenPath + QDir::separator() + file);
        ugenFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QXmlStreamReader xml(&ugenFile);
        Ugen newugen;
        if (xml.readNextStartElement()) {
            if (xml.name() == "ugen" && xml.attributes().value("version") == "1.0") {
                while (xml.readNextStartElement()) {
                    if (xml.name() == "name") {
                        newugen.name = xml.readElementText();
                    } else if (xml.name() == "diskname") {
                        newugen.diskname = xml.readElementText();
                    } else if (xml.name() == "outputs") {

                    } else if (xml.name() == "inputs") {

                    } else if (xml.name() == "parameters") {

                    } else if (xml.name() == "cost") {
                        newugen.cost = xml.readElementText();
                    } else if (xml.name() == "doc") {

                    } else {
                        qDebug() << "XML error skipping: " << xml.name();
                        xml.skipCurrentElement();
                    }
                }
            } else {
                qDebug() << "Unsupported XML for " << file;
            }
        }
        if (xml.error() == QXmlStreamReader::NoError) {
            ugens.append(newugen);
        }
    }
}

bool UgenInterface::isUgen(QString token)
{
    foreach(Ugen ugen, ugens) {
        if (ugen.name == token) {
            return true;
        }
    }
    return false;
}
