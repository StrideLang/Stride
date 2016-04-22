#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

#include "streamplatform.h"
#include "builder.h"
#include "pythonproject.h"


StreamPlatform::StreamPlatform(QStringList platformPaths, QString platform, QString version) :
    m_platformName(platform), m_version(version), m_api(NullPlatform)
{
    QString objectsJson;
    QString typesJson;
    QString functionsJson;
//    QString platformPath = m_platformRootPath + QDir::separator() + m_platformName
//            + QDir::separator() + m_version + QDir::separator() + "plugins";
    platformPaths << "/home/andres/Documents/src/Stride/StreamStack/plugins"; // TODO: un hard-code this
    foreach(QString path, platformPaths) {
        if (m_api != NullPlatform) {
            break; // Stop looking if platform has been found.
        }

        // FIXME move loading somewhere else where it's done less often (Or don't create a new StreamPlatform every time you parse)
        QString fullPath = QDir(path + QDir::separator() + m_platformName
                + QDir::separator() + m_version).absolutePath();
        if (QFile::exists(fullPath)) {
            // First try to find plugin platforms
//            QStringList pluginFiles = QDir(fullPath).entryList(QDir::Files | QDir::NoDotAndDotDot);
//            foreach (QString file, pluginFiles) {
//                if (QLibrary::isLibrary(file)) {
//                    QLibrary pluginLibrary(fullPath + QDir::separator() + file);
//                    if (!pluginLibrary.load()) {
//                        qDebug() << pluginLibrary.errorString();
//                        continue;
//                    }
//                    create_object_t create = (create_object_t) pluginLibrary.resolve("create_object");
//                    platform_name_t get_name = (platform_name_t) pluginLibrary.resolve("platform_name");
//                    platform_version_t get_version = (platform_version_t) pluginLibrary.resolve("platform_version");
//                    if (create && get_name && get_version) {
//                        char name[32];
//                        get_name(name);
//                        double platformVersion = get_version();
//                        //                qDebug() << "Loaded platform " << name << " version " << QString::number(libversion, 'f', 1);
//                        if (m_platformName == QString(name) && (QString::number(platformVersion, 'f', 1) == m_version || m_version == "-1.0")) {
//                            qDebug() << "Using Plugin Platform " << name << " version " << QString::number(platformVersion, 'f', 1);
//                            m_platformPath = fullPath;
//                            m_api = PluginPlatform;
//                            m_pluginName = fullPath + QDir::separator() + file;
////                            QString xmosToolChainRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_14.0.1";
//                            Builder *builder = create("", "", "" );

//                            typesJson = builder->requestTypesJson();
//                            functionsJson = builder->requestFunctionsJson();
//                            objectsJson = builder->requestObjectsJson();
//                            break;
//                        }
//                    }
//                    pluginLibrary.unload();
//                }
            }
            if (m_api != NullPlatform) {
                break; // Stop looking if platform has been found.
            }
            // Now try to find Python platform
            if (QFile::exists(fullPath)) {
                m_library.setLibraryPath(path);
                QStringList nameFilters;
                nameFilters << "*.stride";
                QStringList libraryFiles =  QDir(fullPath).entryList(nameFilters);
                foreach (QString file, libraryFiles) {
                    QString fileName = fullPath + QDir::separator() + file;
                    AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
                    if(tree) {
                        Q_ASSERT(!m_platform.contains(file));
                        m_platform[file] = tree;
                    } else {
                        vector<LangError> errors = AST::getParseErrors();
                        foreach(LangError error, errors) {
                            qDebug() << QString::fromStdString(error.getErrorText());
                        }
                }
                m_platformPath = fullPath;
                m_api = PythonTools;
                m_types = getPlatformTypeNames();
                break;
            }
        }
    }

    if (m_api == NullPlatform) {
        qDebug() << "Platform not found!";
    }
}

StreamPlatform::~StreamPlatform()
{
    foreach(AST *tree, m_platform) {
        tree->deleteChildren();
        delete tree;
    }
}

ListNode *StreamPlatform::getPortsForType(QString typeName)
{
    foreach(AST* nodeGroup, m_platform) {
        foreach(AST *node, nodeGroup->getChildren()) {
            if (node->getNodeType() == AST::Block) {
                BlockNode *block = static_cast<BlockNode *>(node);
                if (block->getObjectType() == "platformType"
                        || block->getObjectType() == "type") {
                    ValueNode *name = static_cast<ValueNode*>(block->getPropertyValue("typeName"));
                    Q_ASSERT(name->getNodeType() == AST::String);
                    if (name->getStringValue() == typeName.toStdString()) {
                        ListNode *portList = getPortsForTypeBlock(block);
                        if (portList) {
                            return portList;
                        }
                    }
                }
            }
        }
    }

    BlockNode * libraryType = m_library.findTypeInLibrary(typeName);
    if (libraryType) {
        ListNode *portList = getPortsForTypeBlock(libraryType);
        if (portList) {
            return portList;
        }
    }

    return NULL;
}

ListNode *StreamPlatform::getPortsForTypeBlock(BlockNode *block)
{
    AST *portsValue = block->getPropertyValue("ports");
    ListNode *portList = NULL;
    if (portsValue) {
        Q_ASSERT(portsValue->getNodeType() == AST::List);
        portList = static_cast<ListNode *>(portsValue);
    }
    AST *inheritedPortsValue = block->getPropertyValue("inherits");
    if (inheritedPortsValue) {
        Q_ASSERT(inheritedPortsValue->getNodeType() == AST::String);
        ListNode *inheritedPortsList = getPortsForType(
                    QString::fromStdString(static_cast<ValueNode *>(inheritedPortsValue)->getStringValue()));
        if (portList) {
            vector<AST *> ports = portList->getChildren();
            vector<AST *> inheritedPorts = inheritedPortsList->getChildren();
            ports.insert(ports.end(), inheritedPorts.begin(), inheritedPorts.end());
            portList->setChildren(ports);
        } else {
            portList = inheritedPortsList;
        }
    }
    return portList;
}

ListNode *StreamPlatform::getPortsForFunction(QString typeName)
{
    foreach(AST* group, m_platform) {
        foreach(AST *node, group->getChildren()){
            if (node->getNodeType() == AST::Block) {
                BlockNode *block = static_cast<BlockNode *>(node);
                if (block->getObjectType() == "platformModule") {
                    if (block->getObjectType() == typeName.toStdString()) {
                        vector<PropertyNode *> ports = block->getProperties();
                        foreach(PropertyNode *port, ports) {
                            if (port->getName() == "ports") {
                                ListNode *portList = static_cast<ListNode *>(port->getValue());
                                Q_ASSERT(portList->getNodeType() == AST::List);
                                return portList;
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

BlockNode *StreamPlatform::getFunction(QString functionName)
{
    QStringList typeNames;
    foreach(AST* node, m_platform) {
        if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(block);
            if (block->getObjectType() == "platformModule"
                    || block->getObjectType() == "module") {
                if (block->getName() == functionName.toStdString()) {
                    return block;
                }
            }
        }
    }
    foreach(AST* node, m_library.getNodes()) {

        if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            if (block->getObjectType() == "module") {
                if (block->getName() == functionName.toStdString()) {
                    return block;
                }
            }
        }
    }
    return NULL;
}

QString StreamPlatform::getPlatformPath()
{
    return m_platformPath;
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
    foreach(AST* group, m_platform) {
        foreach(AST *node, group->getChildren()) {
            if (node->getNodeType() == AST::Block) {
                BlockNode *block = static_cast<BlockNode *>(node);
                if (block->getObjectType() == "platformType") {
                    ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
                    Q_ASSERT(name->getNodeType() == AST::String);
                    typeNames << QString::fromStdString(name->getStringValue());
                }
            }
        }
    }
    vector<AST *> libObjects = m_library.getNodes();
    foreach(AST *node, libObjects) {
        if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(node);
            if (block->getObjectType() == "platformType"
                    || block->getObjectType() == "type") {
                ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
                Q_ASSERT(name->getNodeType() == AST::String);
                typeNames << QString::fromStdString(name->getStringValue());
            }
        }
    }
    return typeNames;
}

QStringList StreamPlatform::getFunctionNames()
{
    QStringList typeNames;
    foreach(AST* node, m_platform) {
        if (node->getNodeType() == AST::Block) {
            BlockNode *block = static_cast<BlockNode *>(block);
            if (block->getObjectType() == "platformModule") {
                typeNames << QString::fromStdString(block->getName());
            }
        }
    }
    return typeNames;
}

Builder *StreamPlatform::createBuilder(QString projectDir)
{
    Builder *builder = NULL;
    if (m_api == StreamPlatform::PythonTools) {
        QString pythonExec = "python";
        builder = new PythonProject(m_platformPath, projectDir, pythonExec);
    } else if(m_api == StreamPlatform::PluginPlatform) {
        QString xmosRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";

        QLibrary pluginLibrary(m_pluginName);
        if (!pluginLibrary.load()) {
            qDebug() << pluginLibrary.errorString();
            return NULL;
        }
        create_object_t create = (create_object_t) pluginLibrary.resolve("create_object");
        if (create) {
            builder = create(m_platformPath, projectDir.toLocal8Bit(), xmosRoot.toLocal8Bit());
        }
        pluginLibrary.unload();
    }
    if (builder && !builder->isValid()) {
        delete builder;
        builder = NULL;
    }
    return builder;
}

QList<AST *> StreamPlatform::getBuiltinObjects()
{
    QList<AST *> objects;
    QMapIterator<QString, AST *> blockGroup(m_platform);
    while (blockGroup.hasNext()) {
        blockGroup.next();
        foreach(AST *element, blockGroup.value()->getChildren()) {
            if (element->getNodeType() == AST::Block) {
                objects << element->deepCopy();
            } else {
                objects << element->deepCopy(); // TODO: This inserts everything. Only insert what is needed
            }
        }
    }
    vector<AST *> libObjects = m_library.getNodes();
    foreach(AST *object, libObjects) {
        objects << object->deepCopy();
    }
    return objects;
}

bool StreamPlatform::isValidType(QString typeName)
{
    if (m_types.contains(typeName)) {
        return true;
    }
    return false;
}

bool StreamPlatform::typeHasPort(QString typeName, QString propertyName)
{
    ListNode *ports = getPortsForType(typeName);
    if (ports) {
        foreach(AST *port, ports->getChildren()) {
            BlockNode *block = static_cast<BlockNode *>(port);
            Q_ASSERT(block->getNodeType() == AST::Block);
            ValueNode *nameValueNode = static_cast<ValueNode *>(block->getPropertyValue("name"));
            Q_ASSERT(nameValueNode->getNodeType() == AST::String);
            if (nameValueNode->getStringValue() == propertyName.toStdString()) {
                return true;
            }
        }
    }
    return false;
}

bool StreamPlatform::isValidPortType(QString typeName, QString propertyName, PortType propType)
{
    BlockNode *block = getFunction(typeName);
    if (!block) {
        return false;
    }
    AST *value = block->getPropertyValue(propertyName.toStdString());
    QString type;
    switch (value->getNodeType()) {
    case AST::Int:
        if (propType == ConstInt) {
            return true;
        } else {
            return false;
        }
    case AST::Real:
        if (propType == ConstReal) {
            return true;
        } else {
            return false;
        }
    case AST::String:
        if (propType == ConstString) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

