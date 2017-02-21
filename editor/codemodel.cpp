#include <QMutexLocker>
#include <QTemporaryFile>
#include <QVector>

#include "codemodel.hpp"

#include "codevalidator.h"
#include "declarationnode.h"
#include "valuenode.h"
#include "listnode.h"
#include "blocknode.h"

CodeModel::CodeModel(QObject *parent) :
    QObject(parent),
    m_lastValidTree(NULL)
{

}

CodeModel::~CodeModel()
{
    QMutexLocker locker(&m_validTreeLock);
    if (m_lastValidTree) {
        foreach(AST * node, m_lastValidTree->getChildren()) {
            node->deleteChildren();
            delete node;
        }
    }
}

QString CodeModel::getHtmlDocumentation(QString symbol)
{
    if (!m_lastValidTree) {
        return tr("Parsing error. Can't update tree.");
    }
    QList<LangError> errors;
    if (symbol[0].toLower() == symbol[0]) {
        QMutexLocker locker(&m_validTreeLock);
        DeclarationNode *typeBlock = CodeValidator::findTypeDeclarationByName(symbol, QVector<AST *>(), m_lastValidTree, errors);
        if (typeBlock) {
            AST *metaValue = typeBlock->getPropertyValue("meta");
            if (metaValue) {
                Q_ASSERT(metaValue);
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                QString docHtml = "<h1>" + symbol + "</h1>\n";
                docHtml += QString::fromStdString(static_cast<ValueNode *>(metaValue)->getStringValue());
                vector<PropertyNode *> properties = typeBlock->getProperties();
                QString propertiesTable = "<table><tr><td><b>Name</b></td><td><b>Type</b></td><td><b>Default</b></td><td><b>Direction</b></td></tr>";
                QString propertiesHtml = tr("<h2>Ports</h2>") + "\n";
                QVector<AST *> ports = CodeValidator::getPortsForTypeBlock(typeBlock, QVector<AST *>(), m_lastValidTree);
                foreach(AST *port, ports) {
                    DeclarationNode *portBlock = static_cast<DeclarationNode *>(port);
                    Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                    if (portBlock->getNodeType() == AST::Declaration) {
                        QString portName = QString::fromStdString(
                                    static_cast<ValueNode *>(portBlock->getPropertyValue("name"))->getStringValue());
                        if (portName != "inherits" && portName != "meta") {
                            AST *portMetaNode = portBlock->getPropertyValue("meta");
                            QString portMeta;
                            if (portMetaNode) {
                                portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
                            }
                            propertiesHtml += "<h3>" + portName + "</h3>" + portMeta;
                            propertiesTable += "<tr><td>" + portName;
                            AST *portTypesValue = portBlock->getPropertyValue("types");
                            Q_ASSERT(portTypesValue);
                            Q_ASSERT(portTypesValue->getNodeType() == AST::List);
                            if (portTypesValue && portTypesValue->getNodeType() == AST::List) {
                                ListNode *validTypesList = static_cast<ListNode *>(portTypesValue);
                                QString typesText;
                                foreach(AST *validTypeNode, validTypesList->getChildren()) {
                                    if (validTypeNode->getNodeType() == AST::String) {
                                        string typeName = static_cast<ValueNode *>(validTypeNode)->getStringValue();
                                        typesText += QString::fromStdString(typeName + ", ");
                                    } else if (validTypeNode->getNodeType() == AST::Block) {
                                        string typeName = static_cast<BlockNode *>(validTypeNode)->getName();
                                        typesText += QString::fromStdString(typeName + ", ");
                                    } else {
                                        typesText += "  ";
                                    }

                                }
                                typesText.chop(2);
                                propertiesTable += "<td>" + typesText + "</td>";
                            }
                            propertiesTable += "</tr>";
                        }
                    }
                }
                propertiesTable += "</table>";
                return docHtml + propertiesHtml + propertiesTable;
            }

        }

    } else if (symbol[0].toUpper() == symbol[0]) { // Check if it is a declared module
        QMutexLocker locker(&m_validTreeLock);
        DeclarationNode *declaration = CodeValidator::findDeclaration(symbol, QVector<AST *>(), m_lastValidTree);
        if (declaration) {
            AST *metaValue = declaration->getPropertyValue("meta");
            Q_ASSERT(metaValue);
            if (metaValue) {
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                QString docHtml = "<h1>" + symbol + "</h1>\n";
                docHtml += QString::fromStdString(static_cast<ValueNode *>(metaValue)->getStringValue());
                QString propertiesTable = "<table> <tr><td><b>Name</b></td><td><b>Type</b></td><td><b>Default</b></td><td><b>Direction</b></td></tr>";
                QString propertiesHtml = tr("<h2>Ports</h2>") + "\n";
                AST *properties = declaration->getPropertyValue("ports");
                if (properties && properties->getNodeType() == AST::List) {
                    Q_ASSERT(properties->getNodeType() == AST::List);
                    ListNode *propertiesList = static_cast<ListNode *>(properties);
                    foreach(AST *member, propertiesList->getChildren()) {
                        DeclarationNode *portBlock = static_cast<DeclarationNode *>(member);
                        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                        if (portBlock->getNodeType() == AST::Declaration) {
                            QString portName = QString::fromStdString(
                                        static_cast<ValueNode *>(portBlock->getPropertyValue("name"))->getStringValue());
                            if (portName != "inherits" && portName != "meta") {
                                AST *portMetaNode = portBlock->getPropertyValue("meta");
                                QString portMeta;
                                if (portMetaNode) {
                                    portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
                                }
                                propertiesHtml += "<h3>" + portName + "</h3>" + portMeta;
//                                propertiesTable += "<tr><td>" + portName;
//                                AST *portTypesValue = portBlock->getPropertyValue("types");
//                                Q_ASSERT(portTypesValue);
//                                Q_ASSERT(portTypesValue->getNodeType() == AST::List);
//                                if (portTypesValue && portTypesValue->getNodeType() == AST::List) {
//                                    ListNode *validTypesList = static_cast<ListNode *>(portTypesValue);
//                                    foreach(AST *validTypeNode, validTypesList->getChildren()) {
//                                        if (validTypeNode->getNodeType() == AST::String) {
//                                            string typeName = static_cast<ValueNode *>(validTypeNode)->getStringValue();
//                                            propertiesTable += QString::fromStdString("<td>" + typeName + "</td>");
//                                        } else if (validTypeNode->getNodeType() == AST::Block) {
//                                            string typeName = static_cast<NameNode *>(validTypeNode)->getName();
//                                            propertiesTable += QString::fromStdString("<td>" + typeName + "</td>");
//                                        } else {
//                                            propertiesTable += "<td>---</td>";
//                                        }
//                                    }
//                                }
//                                propertiesTable += "</tr>";
                            }
                        }
                    }
                    propertiesTable += "</table>";
                    return docHtml + propertiesHtml + propertiesTable;
                }
            }

        }
    }
    return QString();
}

QString CodeModel::getTooltipText(QString symbol)
{
    QString text;
    if (symbol[0].toUpper() == symbol[0]) { // Check if it is a declared module
        QMutexLocker locker(&m_validTreeLock);
        DeclarationNode *declaration = CodeValidator::findDeclaration(symbol, QVector<AST *>(), m_lastValidTree);
        if (declaration) {
            AST *metaValue = declaration->getPropertyValue("meta");
            if (metaValue) {
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                AST *properties = declaration->getPropertyValue("ports");
                if (properties && properties->getNodeType() == AST::List) {
                    text += "<b>" + symbol + "</b>\n(";
                    Q_ASSERT(properties->getNodeType() == AST::List);
                    ListNode *propertiesList = static_cast<ListNode *>(properties);
                    foreach(AST *member, propertiesList->getChildren()) {
                        DeclarationNode *portBlock = static_cast<DeclarationNode *>(member);
                        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                        if (portBlock->getNodeType() == AST::Declaration) {
                            QString portName = QString::fromStdString(
                                        static_cast<ValueNode *>(portBlock->getPropertyValue("name"))->getStringValue());
                            if (portName != "inherits" && portName != "meta") {
                                AST *portMetaNode = portBlock->getPropertyValue("meta");
                                QString portMeta;
                                if (portMetaNode) {
                                    portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
                                }
                                text += "<i>" + portName + "</i>:" + portMeta + "\n";
                            }
                        }
                    }
                    text += ")";
                }
            }
        }
    } else { // word starts with lower case letter
        QList<LangError> errors;
        DeclarationNode *typeBlock = CodeValidator::findTypeDeclarationByName(symbol, QVector<AST *>(), m_lastValidTree, errors);
        if (typeBlock) {
            text = "type: " + symbol;
        }
    }
    return text;
}

QPair<QString, int> CodeModel::getSymbolLocation(QString symbol)
{
    QPair<QString, int> location;
    if (!m_lastValidTree) {
        return location;
    }

    QMutexLocker locker(&m_validTreeLock);
    foreach(AST *node, m_lastValidTree->getChildren()) {
        if (node->getNodeType() == AST::Declaration ||
                node->getNodeType() == AST::BundleDeclaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(node);
            if (block->getName() == symbol.toStdString()) {
                QString fileName = QString::fromStdString(block->getFilename());
                location.first = fileName;
                location.second = block->getLine();
                return location;
            }
            if (block->getObjectType() == "type") {
                AST *namePropertyValue = block->getPropertyValue("typeName");
                Q_ASSERT(namePropertyValue);
                if (namePropertyValue->getNodeType() == AST::String) {
                    ValueNode *nameValue = static_cast<ValueNode *>(namePropertyValue);
                    if (nameValue->getStringValue() == symbol.toStdString()) {
                        QString fileName = QString::fromStdString(block->getFilename());
                        location.first = fileName;
                        location.second = block->getLine();
                        return location;
                    }
                }
            }
        }
    }
    return location;
}

AST * CodeModel::getOptimizedTree()
{
    QMutexLocker locker(&m_validTreeLock);
    AST * optimizedTree = NULL;
    if (m_lastValidTree) {
        optimizedTree = new AST;
        foreach(AST *node, m_lastValidTree->getChildren()) {
            optimizedTree->addChild(node->deepCopy());
        }
    }
    return optimizedTree;
}

Builder *CodeModel::createBuilder(QString projectDir)
{
    QMutexLocker locker(&m_validTreeLock);
    return m_platform.createBuilder(projectDir);
}

QStringList CodeModel::getTypes()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_types;
}

QStringList CodeModel::getFunctions()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_funcs;
}

QStringList CodeModel::getObjectNames()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_objectNames;
}

QString CodeModel::getFunctionSyntax(QString symbol)
{
    if (symbol.isEmpty()) {
        return "";
    }
    QString text;
    if (symbol[0].toUpper() == symbol[0]) { // Check if it is a declared module
        QMutexLocker locker(&m_validTreeLock);
        DeclarationNode *declaration = CodeValidator::findDeclaration(symbol, QVector<AST *>(), m_lastValidTree);
        if (declaration) {
            AST *metaValue = declaration->getPropertyValue("meta");
            Q_ASSERT(metaValue);
            if (metaValue) {
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                AST *properties = declaration->getPropertyValue("ports");
                if (properties && properties->getNodeType() == AST::List) {
                    text += symbol +  "(";
                    Q_ASSERT(properties->getNodeType() == AST::List);
                    ListNode *propertiesList = static_cast<ListNode *>(properties);
                    foreach(AST *member, propertiesList->getChildren()) {
                        DeclarationNode *portBlock = static_cast<DeclarationNode *>(member);
                        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                        if (portBlock->getNodeType() == AST::Declaration) {
                            QString portName = QString::fromStdString(
                                        static_cast<ValueNode *>(portBlock->getPropertyValue("name"))->getStringValue());
                            if (portName != "inherits" && portName != "meta") {
//                                AST *portMetaNode = portBlock->getPropertyValue("meta");
//                                QString portMeta;
//                                if (portMetaNode) {
//                                    portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
//                                }
                                QString defaultValue;
                                AST *portDefaultNode = portBlock->getPropertyValue("default");
                                if (portDefaultNode) {
                                    ValueNode * valueNode = static_cast<ValueNode *>(portDefaultNode);
                                    defaultValue = QString::fromStdString(valueNode->toString());
                                }
                                text += portName + ":" + defaultValue + " ";
                            }
                        }
                    }
                    text += ")";
                }
            }
        }
    }
    return text;
}

QList<LangError> CodeModel::getErrors()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_errors;
}

void CodeModel::updateCodeAnalysis(QString code, QString platformRootPath)
{
    QMutexLocker locker(&m_validTreeLock);
    QTemporaryFile tmpFile;
    if (tmpFile.open()) {
        tmpFile.write(code.toLocal8Bit());
        tmpFile.close();
        AST *tree;
        tree = AST::parseFile(tmpFile.fileName().toLocal8Bit().constData());

        if (tree) {
            CodeValidator validator(platformRootPath, tree);
            validator.validate();
            m_types = validator.getPlatform()->getPlatformTypeNames();
            m_funcs = validator.getPlatform()->getFunctionNames();
            QList<AST *> objects = validator.getPlatform()->getBuiltinObjects();
            m_objectNames.clear();
            foreach (AST *platObject, objects) {
                if (platObject->getNodeType() == AST::Block) {
                    m_objectNames << QString::fromStdString(static_cast<BlockNode *>(platObject)->getName());
                }
            }
            m_errors = validator.getErrors();
            m_platform = *validator.getPlatform();

            if(m_lastValidTree) {
                m_lastValidTree->deleteChildren();
                delete m_lastValidTree;
            }
            m_lastValidTree = tree;
        } else { // !tree
            vector<LangError> syntaxErrors = AST::getParseErrors();
            m_errors.clear();
            for (unsigned int i = 0; i < syntaxErrors.size(); i++) {
                m_errors << syntaxErrors[i];
            }
        }
    }
}
