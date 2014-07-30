#include "ugeninterface.h"

#include <QXmlStreamReader>
#include <QDir>
#include <QFile>
#include <QDebug>

UgenInterface::UgenInterface(QString type, QObject *parent) :
    QObject(parent)
{
    QString ugenPath = "/home/andres/Documents/src/XMOS/Odo/OdoEdit/Ugens";
    QDir ugenBaseDir(ugenPath);
    QStringList ugenDirs = ugenBaseDir.entryList(QDir::Dirs | QDir::NoDot | QDir::NoDotDot);

    foreach(QString dir, ugenDirs) {
        QDir ugenDir(ugenPath + QDir::separator() + dir);
        QStringList ugenFiles = ugenDir.entryList(QStringList() << "*.xml", QDir::Files);

        qDebug() << "UgenInterface intialize";

        foreach(QString file, ugenFiles) {
            qDebug() << "Parsing folder " << dir;
            QFile ugenFile(ugenDir.absolutePath() + QDir::separator() + file);
            ugenFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QXmlStreamReader xml(&ugenFile);
            Ugen newugen;
            if (xml.readNextStartElement()) {
                if (xml.name() == "ugen" && xml.attributes().value("version") == "1.0") {
                    while (xml.readNextStartElement()) {
                        if (xml.name() == "name") {
                            newugen.name = xml.readElementText();
                        } else if (xml.name() == "type") {
                            newugen.type = xml.readElementText();
                        }  else if (xml.name() == "diskname") {
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
                if (newugen.type == type) {
                    ugens.append(newugen);
                    qDebug() << "Added ugen: " << newugen.name;
                } else {
                    qDebug() << "Wrong Ugen type " << newugen.type << " for " << file;
                }
            } else {
                qDebug() << "XML error for " << file;
            }
        }

    }
    qDebug() << "UgenInterface intialize done.";
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
